#!/bin/bash
# run-all-tests.sh - Ejecuta todas las pruebas del VFS

echo "======================================"
echo "EJECUTANDO TODAS LAS PRUEBAS DEL VFS"
echo "======================================"
echo

# Colores
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Verificar que estamos en el directorio correcto
if [ ! -f "Makefile" ] || [ ! -d "src" ]; then
    echo -e "${RED}Error: Ejecuta este script desde el directorio raíz del proyecto${NC}"
    exit 1
fi

# Limpiar y compilar
echo -e "${YELLOW}1. Limpiando y compilando...${NC}"
make cleanall > /dev/null 2>&1
make 
make test
if [ $? -ne 0 ]; then
    echo -e "${RED}Error en la compilación${NC}"
    exit 1
fi
echo -e "${GREEN}✓ Compilación exitosa${NC}"
echo

# Preguntar si ejecutar suite completa
echo -e "${YELLOW}¿Deseas ejecutar la suite completa de pruebas? (s/n)${NC}"
read -r respuesta

if [[ "$respuesta" == "s" || "$respuesta" == "S" ]]; then
    echo
    echo -e "${YELLOW}3. Ejecutando suite completa de pruebas...${NC}"
    echo "=========================================="
    ./test-vfs-suite
    
    # Capturar código de salida
    TEST_RESULT=$?
    
    echo
    if [ $TEST_RESULT -eq 0 ]; then
        echo -e "${GREEN}✓ TODAS LAS PRUEBAS PASARON EXITOSAMENTE${NC}"
    else
        echo -e "${RED}✗ Algunas pruebas fallaron${NC}"
    fi
else
    echo -e "${YELLOW}Suite completa omitida${NC}"
fi

# Limpiar archivos temporales
echo
echo -e "${YELLOW}4. Limpiando archivos temporales...${NC}"
rm -f test_*.txt archivo*.txt temp*.txt *.dat *.bin cat_*.txt order_test.txt empty_cat.txt trunc_test.txt recovered*.txt
echo -e "${GREEN}✓ Limpieza completada${NC}"

echo
echo "======================================"
echo "FIN DE LAS PRUEBAS"
echo "======================================"

# Mostrar imágenes disponibles para inspección
echo
echo "Imágenes de filesystem disponibles para inspección:"
ls -lh *.img 2>/dev/null || echo "(ninguna)"

echo
echo "Usa './vfs-info <imagen>' para ver detalles del filesystem"
echo "Usa './vfs-ls <imagen>' para ver archivos"