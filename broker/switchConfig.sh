helpFunction()
{
   echo ""
   echo "Como se usa: $0 nombreConfig , donde nombreConfig puede ser:"
   echo -e "\tcons1"
   echo -e "\tcons2"
   echo -e "\tcomp1"
   echo -e "\tcomp2"
   echo -e "\tbs1"
   echo -e "\tbs2"
   echo -e "\tcompleta1"
   echo -e "\tcompleta2"
   exit 1 # Exit script after printing help
}

if [ -n "$1" ]
then
    truncate -s 0 ./cfg/broker.config
    echo -e "IP_BROKER=127.0.0.1\nLOG_FILE=./cfg/obligatory.log\nOPTIONAL_LOG_FILE=./cfg/optional.log\nLOG_SHOW=1\nOPTIONAL_LOG_SHOW=0\nDUMP_FILE=./cfg/cache.tmp\nPUERTO_BROKER=6009" >>  ./cfg/broker.config
fi

case $1 in
    cons1)
        echo -e "Seteando config de Consolidacion de Particiones Parte 1"
        echo -e "TAMANO_MEMORIA=64\nTAMANO_MINIMO_PARTICION=4\nALGORITMO_MEMORIA=PARTICIONES\nALGORITMO_REEMPLAZO=FIFO\nALGORITMO_PARTICION_LIBRE=FF\nFRECUENCIA_COMPACTACION=10" >>  ./cfg/broker.config
        ;;
    cons2)
        echo -e "Seteando config de Consolidacion de Particiones Parte 2"
        echo -e "TAMANO_MEMORIA=64\nTAMANO_MINIMO_PARTICION=4\nALGORITMO_MEMORIA=PARTICIONES\nALGORITMO_REEMPLAZO=LRU\nALGORITMO_PARTICION_LIBRE=FF\nFRECUENCIA_COMPACTACION=10" >>  ./cfg/broker.config
        ;;
    comp1)
        echo -e "Seteando config de Compactacion de Particiones Parte 1"
        echo -e "TAMANO_MEMORIA=64\nTAMANO_MINIMO_PARTICION=4\nALGORITMO_MEMORIA=PARTICIONES\nALGORITMO_REEMPLAZO=FIFO\nALGORITMO_PARTICION_LIBRE=FF\nFRECUENCIA_COMPACTACION=1" >>  ./cfg/broker.config
        ;;
    comp2)
        echo -e "Seteando config de Compactación de Particiones Parte 2"
        echo -e "TAMANO_MEMORIA=64\nTAMANO_MINIMO_PARTICION=4\nALGORITMO_MEMORIA=PARTICIONES\nALGORITMO_REEMPLAZO=LRU\nALGORITMO_PARTICION_LIBRE=FF\nFRECUENCIA_COMPACTACION=1" >>  ./cfg/broker.config
        ;;
    bs1)
        echo -e "Seteando config de Buddy System Parte 1"
        echo -e "TAMANO_MEMORIA=64\nTAMANO_MINIMO_PARTICION=4\nALGORITMO_MEMORIA=BS\nALGORITMO_REEMPLAZO=FIFO\nALGORITMO_PARTICION_LIBRE=FF\nFRECUENCIA_COMPACTACION=1" >>  ./cfg/broker.config
        ;;
    bs2)
        echo -e "Seteando config de Buddy System Parte 2"
        echo -e "TAMANO_MEMORIA=64\nTAMANO_MINIMO_PARTICION=4\nALGORITMO_MEMORIA=BS\nALGORITMO_REEMPLAZO=LRU\nALGORITMO_PARTICION_LIBRE=FF\nFRECUENCIA_COMPACTACION=1" >>  ./cfg/broker.config
        ;;
    completa1)
        echo -e "Seteando config de Prueba Integradora 1"
        echo -e "TAMANO_MEMORIA=64\nTAMANO_MINIMO_PARTICION=4\nALGORITMO_MEMORIA=PARTICIONES\nALGORITMO_REEMPLAZO=FIFO\nALGORITMO_PARTICION_LIBRE=FF\nFRECUENCIA_COMPACTACION=1" >>  ./cfg/broker.config
        ;;
    completa2)
        echo -e "Seteando config de Prueba Integradora 2"
        echo -e "TAMANO_MEMORIA=64\nTAMANO_MINIMO_PARTICION=4\nALGORITMO_MEMORIA=BS\nALGORITMO_REEMPLAZO=FIFO\nALGORITMO_PARTICION_LIBRE=FF\nFRECUENCIA_COMPACTACION=1" >>  ./cfg/broker.config
        ;;
    ? ) helpFunction ;; # Print helpFunction in case parameter is non-existent
esac

# Print helpFunction in case parameters are empty
if [ -z "$1" ]
then
   echo "Falta el nombre de lo que queres cambiar";
   helpFunction
fi