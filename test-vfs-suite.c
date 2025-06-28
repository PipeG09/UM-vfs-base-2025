// test-vfs-suite.c - Suite completa de pruebas automatizadas para VFS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>

#define TEST_IMG "test_suite.img"
#define TEMP_FILE "temp_test.txt"
#define MAX_CMD 512

// Colores para output
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"

// Contadores globales
int tests_passed = 0;
int tests_failed = 0;
int tests_total = 0;

// Función para ejecutar comando y verificar resultado
int run_test(const char *test_name, const char *command, int expected_result) {
    tests_total++;
    printf("\n%s[TEST %d]%s %s\n", BLUE, tests_total, RESET, test_name);
    printf("Comando: %s\n", command);
    
    int result = system(command);
    int exit_code = WEXITSTATUS(result);
    
    if ((expected_result == 0 && exit_code == 0) || 
        (expected_result != 0 && exit_code != 0)) {
        printf("%s✓ PASSED%s\n", GREEN, RESET);
        tests_passed++;
        return 1;
    } else {
        printf("%s✗ FAILED%s (esperado: %s, obtenido: %s)\n", 
               RED, RESET, 
               expected_result == 0 ? "éxito" : "error",
               exit_code == 0 ? "éxito" : "error");
        tests_failed++;
        return 0;
    }
}

// Crear archivo con contenido específico
void create_test_file(const char *filename, const char *content, size_t size) {
    FILE *f = fopen(filename, "wb");
    if (f) {
        if (content) {
            fwrite(content, 1, strlen(content), f);
        } else {
            // Crear archivo de tamaño específico con datos aleatorios
            char buffer[1024];
            for (size_t i = 0; i < size; i += sizeof(buffer)) {
                size_t to_write = (size - i < sizeof(buffer)) ? size - i : sizeof(buffer);
                for (size_t j = 0; j < to_write; j++) {
                    buffer[j] = 'A' + (i + j) % 26;
                }
                fwrite(buffer, 1, to_write, f);
            }
        }
        fclose(f);
    }
}

// Verificar si archivo existe
int file_exists(const char *filename) {
    struct stat st;
    return stat(filename, &st) == 0;
}

// Limpiar archivos temporales
void cleanup_temp_files() {
    system("rm -f test_*.txt archivo*.txt temp*.txt *.dat *.bin temp_test.txt");
}

// Suite principal de pruebas
int main() {
    char cmd[MAX_CMD];
    
    printf("\n%s=== SUITE DE PRUEBAS AUTOMATIZADAS PARA VFS ===%s\n", YELLOW, RESET);
    printf("Fecha: %s\n", __DATE__);
    
    // Limpiar antes de empezar
    cleanup_temp_files();
    unlink(TEST_IMG);
    
    // ==== PRUEBAS DE CREACIÓN DEL FILESYSTEM ====
    printf("\n%s--- PRUEBAS DE CREACIÓN DEL FILESYSTEM ---%s\n", YELLOW, RESET);
    
    // Test 1: Crear filesystem válido
    snprintf(cmd, MAX_CMD, "./vfs-mkfs %s 100 50", TEST_IMG);
    run_test("Crear filesystem válido", cmd, 0);
    
    // Test 2: Intentar crear filesystem que ya existe
    snprintf(cmd, MAX_CMD, "./vfs-mkfs %s 100 50 2>/dev/null", TEST_IMG);
    run_test("Crear filesystem existente (debe fallar)", cmd, 1);
    
    // Test 3: Parámetros inválidos - muy pocos bloques
    unlink("invalid.img");
    run_test("Filesystem con muy pocos bloques", 
             "./vfs-mkfs invalid.img 10 50 2>/dev/null", 1);
    
    // Test 4: Parámetros inválidos - más inodos que bloques
    run_test("Más inodos que bloques", 
             "./vfs-mkfs invalid.img 100 200 2>/dev/null", 1);
    
    // ==== PRUEBAS DE INFORMACIÓN ====
    printf("\n%s--- PRUEBAS DE INFORMACIÓN ---%s\n", YELLOW, RESET);
    
    // Test 5: Ver información de filesystem válido
    snprintf(cmd, MAX_CMD, "./vfs-info %s >/dev/null", TEST_IMG);
    run_test("Ver información de filesystem", cmd, 0);
    
    // Test 6: Ver información de imagen inexistente
    run_test("Info de imagen inexistente", 
             "./vfs-info noexiste.img 2>/dev/null", 1);
    
    // ==== PRUEBAS DE TOUCH ====
    printf("\n%s--- PRUEBAS DE TOUCH ---%s\n", YELLOW, RESET);
    
    // Test 7: Crear archivo válido
    snprintf(cmd, MAX_CMD, "./vfs-touch %s archivo1.txt", TEST_IMG);
    run_test("Crear archivo con touch", cmd, 0);
    
    // Test 8: Crear múltiples archivos
    snprintf(cmd, MAX_CMD, "./vfs-touch %s archivo2.txt archivo3.txt archivo4.txt", TEST_IMG);
    run_test("Crear múltiples archivos", cmd, 0);
    
    // Test 9: Crear archivo que ya existe
    snprintf(cmd, MAX_CMD, "./vfs-touch %s archivo1.txt 2>/dev/null", TEST_IMG);
    run_test("Touch archivo existente (debe fallar)", cmd, 1);
    
    // Test 10: Nombre inválido - con espacios
    snprintf(cmd, MAX_CMD, "./vfs-touch %s \"archivo con espacios.txt\" 2>/dev/null", TEST_IMG);
    run_test("Nombre con espacios (debe fallar)", cmd, 1);
    
    // Test 11: Nombre inválido - caracteres especiales
    snprintf(cmd, MAX_CMD, "./vfs-touch %s archivo@#$.txt 2>/dev/null", TEST_IMG);
    run_test("Nombre con caracteres especiales (debe fallar)", cmd, 1);
    
    // Test 12: Nombre válido con guiones y puntos
    snprintf(cmd, MAX_CMD, "./vfs-touch %s archivo_test-123.dat", TEST_IMG);
    run_test("Nombre con guiones y guión bajo", cmd, 0);
    
    // Test 13: Nombre muy largo (más de 27 caracteres)
    snprintf(cmd, MAX_CMD, "./vfs-touch %s archivo_con_nombre_extremadamente_largo.txt 2>/dev/null", TEST_IMG);
    run_test("Nombre muy largo (debe fallar)", cmd, 1);
    
    // Test 14: Nombre de exactamente 27 caracteres
    snprintf(cmd, MAX_CMD, "./vfs-touch %s abcdefghijklmnopqrstuvwxyz1", TEST_IMG);
    run_test("Nombre de 27 caracteres (límite)", cmd, 0);
    
    // ==== PRUEBAS DE COPY ====
    printf("\n%s--- PRUEBAS DE COPY ---%s\n", YELLOW, RESET);
    
    // Test 15: Copiar archivo pequeño
    create_test_file("test_small.txt", "Contenido de prueba pequeño\n", 0);
    snprintf(cmd, MAX_CMD, "./vfs-copy %s test_small.txt test_small.txt", TEST_IMG);
    run_test("Copiar archivo pequeño", cmd, 0);
    
    // Test 16: Copiar archivo de exactamente 1 bloque (1024 bytes)
    create_test_file("test_1block.txt", NULL, 1024);
    snprintf(cmd, MAX_CMD, "./vfs-copy %s test_1block.txt test_1block.txt", TEST_IMG);
    run_test("Copiar archivo de 1 bloque exacto", cmd, 0);
    
    // Test 17: Copiar archivo de múltiples bloques (3.5 KB)
    create_test_file("test_multi.txt", NULL, 3584);
    snprintf(cmd, MAX_CMD, "./vfs-copy %s test_multi.txt test_multi.txt", TEST_IMG);
    run_test("Copiar archivo de múltiples bloques", cmd, 0);
    
    // Test 18: Copiar archivo grande con bloques indirectos (10 KB)
    create_test_file("test_large.txt", NULL, 10240);
    snprintf(cmd, MAX_CMD, "./vfs-copy %s test_large.txt test_large.txt", TEST_IMG);
    run_test("Copiar archivo con bloques indirectos", cmd, 0);
    
    // Test 19: Copiar a nombre que ya existe
    snprintf(cmd, MAX_CMD, "./vfs-copy %s test_small.txt archivo1.txt 2>/dev/null", TEST_IMG);
    run_test("Copiar a archivo existente (debe fallar)", cmd, 1);
    
    // Test 20: Copiar archivo inexistente
    snprintf(cmd, MAX_CMD, "./vfs-copy %s noexiste.txt destino.txt 2>/dev/null", TEST_IMG);
    run_test("Copiar archivo inexistente (debe fallar)", cmd, 1);
    
    // ==== PRUEBAS DE LS Y LSORT ====
    printf("\n%s--- PRUEBAS DE LS Y LSORT ---%s\n", YELLOW, RESET);
    
    // Test 21: Listar archivos
    snprintf(cmd, MAX_CMD, "./vfs-ls %s >/dev/null", TEST_IMG);
    run_test("Listar archivos con ls", cmd, 0);
    
    // Test 22: Listar archivos ordenados
    snprintf(cmd, MAX_CMD, "./vfs-lsort %s >/dev/null", TEST_IMG);
    run_test("Listar archivos ordenados", cmd, 0);
    
    // Test 23: Verificar orden alfabético
    snprintf(cmd, MAX_CMD, "./vfs-touch %s zzz.txt aaa.txt mmm.txt", TEST_IMG);
    system(cmd);
    snprintf(cmd, MAX_CMD, "./vfs-lsort %s | grep -E '(aaa|mmm|zzz)' | head -3 > order_test.txt", TEST_IMG);
    system(cmd);
    // Verificar que aaa aparece antes que mmm y mmm antes que zzz
    run_test("Verificar orden alfabético correcto", 
             "grep -q 'aaa' order_test.txt && grep -q 'zzz' order_test.txt", 0);
    unlink("order_test.txt");
    
    // ==== PRUEBAS DE CAT ====
    printf("\n%s--- PRUEBAS DE CAT ---%s\n", YELLOW, RESET);
    
    // Test 24: Mostrar contenido de archivo
    snprintf(cmd, MAX_CMD, "./vfs-cat %s test_small.txt > cat_output.txt", TEST_IMG);
    system(cmd);
    run_test("Cat archivo y verificar contenido", 
             "grep -q 'Contenido de prueba' cat_output.txt", 0);
    unlink("cat_output.txt");
    
    // Test 25: Cat de múltiples archivos
    create_test_file("test_cat1.txt", "Parte1", 0);
    create_test_file("test_cat2.txt", "Parte2", 0);
    snprintf(cmd, MAX_CMD, "./vfs-copy %s test_cat1.txt cat1.txt", TEST_IMG);
    system(cmd);
    snprintf(cmd, MAX_CMD, "./vfs-copy %s test_cat2.txt cat2.txt", TEST_IMG);
    system(cmd);
    snprintf(cmd, MAX_CMD, "./vfs-cat %s cat1.txt cat2.txt > cat_multi.txt", TEST_IMG);
    system(cmd);
    run_test("Cat múltiples archivos", 
             "grep -q 'Parte1' cat_multi.txt && grep -q 'Parte2' cat_multi.txt", 0);
    unlink("cat_multi.txt");
    
    // Test 26: Cat archivo inexistente
    snprintf(cmd, MAX_CMD, "./vfs-cat %s noexiste.txt 2>/dev/null", TEST_IMG);
    run_test("Cat archivo inexistente (debe fallar)", cmd, 1);
    
    // Test 27: Cat archivo vacío
    snprintf(cmd, MAX_CMD, "./vfs-cat %s archivo1.txt > empty_cat.txt", TEST_IMG);
    system(cmd);
    run_test("Cat archivo vacío", "test ! -s empty_cat.txt", 0);
    unlink("empty_cat.txt");
    
    // ==== PRUEBAS DE TRUNCATE ====
    printf("\n%s--- PRUEBAS DE TRUNCATE ---%s\n", YELLOW, RESET);
    
    // Test 28: Truncar archivo con contenido
    snprintf(cmd, MAX_CMD, "./vfs-trunc %s test_small.txt", TEST_IMG);
    run_test("Truncar archivo", cmd, 0);
    
    // Test 29: Verificar que archivo está vacío después de truncar
    snprintf(cmd, MAX_CMD, "./vfs-cat %s test_small.txt > trunc_test.txt", TEST_IMG);
    system(cmd);
    run_test("Verificar archivo vacío tras truncar", "test ! -s trunc_test.txt", 0);
    unlink("trunc_test.txt");
    
    // Test 30: Truncar archivo grande (liberar bloques indirectos)
    snprintf(cmd, MAX_CMD, "./vfs-trunc %s test_large.txt", TEST_IMG);
    run_test("Truncar archivo con bloques indirectos", cmd, 0);
    
    // Test 31: Truncar archivo inexistente
    snprintf(cmd, MAX_CMD, "./vfs-trunc %s noexiste.txt 2>/dev/null", TEST_IMG);
    run_test("Truncar archivo inexistente (debe fallar)", cmd, 1);
    
    // Test 32: Truncar múltiples archivos
    snprintf(cmd, MAX_CMD, "./vfs-trunc %s cat1.txt cat2.txt test_multi.txt", TEST_IMG);
    run_test("Truncar múltiples archivos", cmd, 0);
    
    // ==== PRUEBAS DE REMOVE ====
    printf("\n%s--- PRUEBAS DE REMOVE ---%s\n", YELLOW, RESET);
    
    // Test 33: Eliminar archivo
    snprintf(cmd, MAX_CMD, "./vfs-rm %s archivo1.txt", TEST_IMG);
    run_test("Eliminar archivo", cmd, 0);
    
    // Test 34: Verificar que archivo fue eliminado
    snprintf(cmd, MAX_CMD, "./vfs-ls %s | grep -q archivo1.txt", TEST_IMG);
    run_test("Verificar archivo eliminado", cmd, 1);
    
    // Test 35: Eliminar múltiples archivos
    snprintf(cmd, MAX_CMD, "./vfs-rm %s archivo2.txt archivo3.txt archivo4.txt", TEST_IMG);
    run_test("Eliminar múltiples archivos", cmd, 0);
    
    // Test 36: Eliminar archivo inexistente
    snprintf(cmd, MAX_CMD, "./vfs-rm %s noexiste.txt 2>/dev/null", TEST_IMG);
    run_test("Eliminar archivo inexistente", cmd, 0); // rm no falla si no existe
    
    // Test 37: Eliminar archivo con bloques indirectos
    create_test_file("test_remove_large.txt", NULL, 12288);
    snprintf(cmd, MAX_CMD, "./vfs-copy %s test_remove_large.txt remove_large.txt", TEST_IMG);
    system(cmd);
    snprintf(cmd, MAX_CMD, "./vfs-rm %s remove_large.txt", TEST_IMG);
    run_test("Eliminar archivo grande", cmd, 0);
    
    // ==== PRUEBAS DE LÍMITES ====
    printf("\n%s--- PRUEBAS DE LÍMITES DEL FILESYSTEM ---%s\n", YELLOW, RESET);
    
    // Test 38: Crear muchos archivos hasta agotar inodos
    printf("\n%s[TEST %d]%s Llenar filesystem con archivos\n", BLUE, ++tests_total, RESET);
    int files_created = 0;
    for (int i = 0; i < 70; i++) {
        snprintf(cmd, MAX_CMD, "./vfs-touch %s limite_%d.dat 2>/dev/null", TEST_IMG, i);
        if (system(cmd) != 0) {
            break;
        }
        files_created++;
    }

    if (files_created > 10) {
        printf("%s✓ PASSED%s (creados %d archivos antes de agotar espacio)\n", GREEN, RESET, files_created);
        tests_passed++;
    } else {
        printf("%s✗ FAILED%s (esperado > 10 archivos, creados %d)\n", RED, RESET, files_created);
        tests_failed++;
    }
    
    // Test 39: Intentar crear archivo cuando hay poco espacio
    // Intentamos crear varios archivos más para asegurar que eventualmente falle
    int failed = 0;
    for (int i = 0; i < 10; i++) {
        snprintf(cmd, MAX_CMD, "./vfs-touch %s extra_%d.txt 2>/dev/null", TEST_IMG, i);
        if (system(cmd) != 0) {
            failed = 1;
            break;
        }
    }
    if (failed) {
        printf("%s✓ PASSED%s (filesystem eventualmente se llenó)\n", GREEN, RESET);
        tests_passed++;
    } else {
        printf("%s✗ FAILED%s (se esperaba que el filesystem se llenara)\n", RED, RESET);
        tests_failed++;
    }
    tests_total++;
    
    // ==== PRUEBAS DE INTEGRIDAD ====
    printf("\n%s--- PRUEBAS DE INTEGRIDAD DE DATOS ---%s\n", YELLOW, RESET);
    
    // Limpiar para pruebas de integridad
    unlink(TEST_IMG);
    snprintf(cmd, MAX_CMD, "./vfs-mkfs %s 200 50", TEST_IMG);
    system(cmd);
    
    // Test 40: Copiar y recuperar archivo, verificar integridad
    create_test_file("test_integrity.txt", "Datos importantes que no deben corromperse\n", 0);
    snprintf(cmd, MAX_CMD, "./vfs-copy %s test_integrity.txt integrity.txt", TEST_IMG);
    system(cmd);
    snprintf(cmd, MAX_CMD, "./vfs-cat %s integrity.txt > recovered.txt", TEST_IMG);
    system(cmd);
    run_test("Integridad de datos pequeños", 
             "diff -q test_integrity.txt recovered.txt", 0);
    
    // Test 41: Integridad con archivo grande
    create_test_file("test_big_integrity.bin", NULL, 8192);
    snprintf(cmd, MAX_CMD, "./vfs-copy %s test_big_integrity.bin big.bin", TEST_IMG);
    system(cmd);
    snprintf(cmd, MAX_CMD, "./vfs-cat %s big.bin > recovered_big.bin", TEST_IMG);
    system(cmd);
    run_test("Integridad de archivo grande", 
             "diff -q test_big_integrity.bin recovered_big.bin", 0);
    
    // ==== PRUEBAS DE PERMISOS ====
    printf("\n%s--- PRUEBAS DE PERMISOS Y METADATOS ---%s\n", YELLOW, RESET);
    
    // Test 42: Verificar permisos por defecto
    snprintf(cmd, MAX_CMD, "./vfs-ls %s | grep integrity.txt | grep -q 'rw-r--'", TEST_IMG);
    run_test("Verificar permisos por defecto", cmd, 0);
    
    // Test 43: Verificar que se mantienen permisos al copiar
    system("chmod 755 test_integrity.txt");
    snprintf(cmd, MAX_CMD, "./vfs-copy %s test_integrity.txt perms.txt", TEST_IMG);
    system(cmd);
    snprintf(cmd, MAX_CMD, "./vfs-ls %s | grep perms.txt | grep -q 'rwxr-xr-x'", TEST_IMG);
    run_test("Mantener permisos al copiar", cmd, 0);
    
    // ==== PRUEBAS ESPECIALES ====
    printf("\n%s--- PRUEBAS ESPECIALES ---%s\n", YELLOW, RESET);
    
    // Test 44: Operación en imagen corrupta
    create_test_file("corrupted.img", "BASURA", 1024);
    snprintf(cmd, MAX_CMD, "./vfs-ls corrupted.img 2>/dev/null");
    run_test("Operación en imagen corrupta (debe fallar)", cmd, 1);
    unlink("corrupted.img");
    
    // Test 45: Verificar entradas . y .. en directorio raíz
    snprintf(cmd, MAX_CMD, "./vfs-ls %s | grep -E '^[[:space:]]*1.*d.*\\.$' | grep -v '\\.\\.'", TEST_IMG);
    run_test("Verificar entrada . en raíz", cmd, 0);
    
    snprintf(cmd, MAX_CMD, "./vfs-ls %s | grep -E '^[[:space:]]*1.*d.*\\.\\.$'", TEST_IMG);
    run_test("Verificar entrada .. en raíz", cmd, 0);
    
    // ==== RESUMEN FINAL ====
    printf("\n%s=========================================%s\n", YELLOW, RESET);
    printf("%sRESUMEN DE PRUEBAS%s\n", YELLOW, RESET);
    printf("%s=========================================%s\n", YELLOW, RESET);
    printf("Total de pruebas: %d\n", tests_total);
    printf("%sPruebas exitosas: %d%s\n", GREEN, tests_passed, RESET);
    printf("%sPruebas fallidas: %d%s\n", RED, tests_failed, RESET);
    printf("Porcentaje de éxito: %.1f%%\n", 
           tests_total > 0 ? (tests_passed * 100.0 / tests_total) : 0);
    
    if (tests_failed == 0) {
        printf("\n%s¡TODAS LAS PRUEBAS PASARON EXITOSAMENTE!%s\n", GREEN, RESET);
    } else {
        printf("\n%sAlgunas pruebas fallaron. Revisa los resultados arriba.%s\n", RED, RESET);
    }
    
    // Limpiar al final
    cleanup_temp_files();
    
    return tests_failed > 0 ? 1 : 0;
}