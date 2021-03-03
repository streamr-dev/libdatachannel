
echo "Before killing"
screen -ls
screen -ls | grep client | cut -d. -f1 | awk '{print $1}' | xargs kill
echo "After killing"
screen -ls
