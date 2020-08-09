helpFunction()
{
   echo ""
   echo "Use: $0 nombreConfig , where nombreConfig could be:"
   echo -e "\tfifo_pi" 
   echo -e "\tfifo_pc"
   echo -e "\trr"
   echo -e "\tsjf_sd"
   echo -e "\tsjf_sd_pc"
   echo -e "\tsjf_cd"

   exit 1 # Exit script after printing help
}

if [ -n "$1" ]
then
    truncate -s 0 ./cfg/team.config
    echo -e "IP_TEAM=127.0.0.2\nPUERTO_TEAM=6011\nPOKEMON_ENTRENADORES=[Pikachu]\nOBJETIVOS_ENTRENADORES=[Pikachu|Squirtle,Pikachu|Gengar,Squirtle|Onix]\nTIEMPO_RECONEXION=30\nRETARDO_CICLO_CPU=5\nESTIMACION_INICIAL=5\nIP_BROKER=127.0.0.1 \nPUERTO_BROKER=6009\nLOG_FILE=./cfg/logObligatorio.log\nLOG_FILE_OPTIONAL=./cfg/logOptional.log\nCONNECTION_TIME=10 " >>  ./cfg/team.config
fi

case $1 in
    fifo_pi)
	echo -e "Setting config to Algorithm FIFO Unit Test Team"
        echo -e "POSICIONES_ENTRENADORES=[1|3,2|3,3|2]\nALGORITMO_PLANIFICACION=FIFO\nQUANTUM=0\nALPHA=0" >>  ./cfg/team.config
        ;;
    fifo_pc)
        echo -e "Setting config to Algorithm FIFO Global Test Team"
        echo -e "POSICIONES_ENTRENADORES=[1|3,2|3,2|2]\nALGORITMO_PLANIFICACION=FIFO\nQUANTUM=0\nALPHA=0" >>  ./cfg/team.config
        ;;
    rr)
        echo -e "Setting config to Algorithm Round Robin Unit Test Team"
        echo -e "POSICIONES_ENTRENADORES=[1|3,2|3,3|2]\nALGORITMO_PLANIFICACION=RR\nQUANTUM=2\nALPHA=0"  >>  ./cfg/team.config
        ;;
    sjf_sd)
        echo -e "Setting config to Algorithm SJF sin desalojo Unit Test Team"
        echo -e "POSICIONES_ENTRENADORES=[1|3,2|3,3|2]\nALGORITMO_PLANIFICACION=SJF-SD\nQUANTUM=0\nALPHA=0.5" >>  ./cfg/team.config
        ;;
    sjf_sd_pc)
        echo -e "Setting config to Algorithm SJF sin desalojo Global Test Team"
        echo -e "POSICIONES_ENTRENADORES=[1|3,2|3,2|2]\nALGORITMO_PLANIFICACION=SJF-SD\nQUANTUM=0\nALPHA=0.5" >>  ./cfg/team.config
        ;;
    sjf_cd)
        echo -e "Setting config to Algorithm SJF con desalojo Unit Test Team"
        echo -e "POSICIONES_ENTRENADORES=[1|3,2|3,3|2]\nALGORITMO_PLANIFICACION=SJF-CD\nQUANTUM=0\nALPHA=0.5" >>  ./cfg/team.config
        ;;
    ? ) helpFunction ;; # Print helpFunction in case parameter is non-existent
esac

# Print helpFunction in case parameters are empty
if [ -z "$1" ]
then
   echo "Choose the algorithm to configure in file";
   helpFunction
fi
