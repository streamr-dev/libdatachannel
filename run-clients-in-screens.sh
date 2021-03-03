
NUM=10
WSURL='ws://95.216.64.56:8080/'
NODE_ID_PREFIX='client'

if [ -z "$1" ]
  then
    echo "No second argument given, running default of ${NUM} clients"
  else
    NUM=$1
fi

if [ -z "$WS_URL" ]
  then
    echo "Environment variable WS_URL not set, starting with default WS_URL of ${WSURL}"
  else
    WSURL=$WS_URL
fi


ulimit -c unlimited
mkdir -p logs
for (( c=1; c<=$NUM; c++ ))
do
   screen -S client${c} -dm bash -c "WS_URL=${WSURL} ./build/examples/multi-node-benchmark/multi-node-benchmark &> logs/client${c}.txt ; exec sh"
done

screen -ls
