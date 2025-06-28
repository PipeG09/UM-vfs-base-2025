#!/bin/bash
# verify-and-fix.sh - Verifica qué versiones están en uso

echo "=== VERIFICACIÓN DE ARCHIVOS ==="
echo

# Verificar si los archivos tienen el patrón correcto
echo "1. Verificando vfs-touch.c..."
if grep -q "int errors = 0;" src/vfs-touch.c 2>/dev/null; then
    echo "✓ vfs-touch.c tiene el contador de errores"
else
    echo "✗ vfs-touch.c NO tiene el contador de errores - usando versión antigua"
fi

echo
echo "2. Verificando vfs-cat.c..."
if grep -q "int errors = 0;" src/vfs-cat.c 2>/dev/null; then
    echo "✓ vfs-cat.c tiene el contador de errores"
else
    echo "✗ vfs-cat.c NO tiene el contador de errores - usando versión antigua"
fi

echo
echo "3. Verificando vfs-trunc.c..."
if grep -q "int errors = 0;" src/vfs-trunc.c 2>/dev/null; then
    echo "✓ vfs-trunc.c tiene el contador de errores"
else
    echo "✗ vfs-trunc.c NO tiene el contador de errores - usando versión antigua"
fi

echo
echo "4. Probando comportamiento real..."
echo

# Crear imagen de prueba
./vfs-mkfs test_verify.img 50 20 >/dev/null 2>&1

# Test 1: touch archivo existente
echo -n "TEST: touch archivo existente... "
./vfs-touch test_verify.img archivo1 >/dev/null 2>&1
./vfs-touch test_verify.img archivo1 >/dev/null 2>&1
if [ $? -ne 0 ]; then
    echo "✓ CORRECTO (retorna error)"
else
    echo "✗ INCORRECTO (no retorna error)"
fi

# Test 2: touch nombre inválido
echo -n "TEST: touch nombre con espacios... "
./vfs-touch test_verify.img "archivo con espacios" >/dev/null 2>&1
if [ $? -ne 0 ]; then
    echo "✓ CORRECTO (retorna error)"
else
    echo "✗ INCORRECTO (no retorna error)"
fi

# Test 3: cat archivo inexistente
echo -n "TEST: cat archivo inexistente... "
./vfs-cat test_verify.img noexiste >/dev/null 2>&1
if [ $? -ne 0 ]; then
    echo "✓ CORRECTO (retorna error)"
else
    echo "✗ INCORRECTO (no retorna error)"
fi

# Limpiar
rm -f test_verify.img

echo
echo "=== CONCLUSIÓN ==="
echo "Si ves errores arriba, necesitas actualizar los archivos src/*.c"
echo "con las versiones corregidas de los artifacts."