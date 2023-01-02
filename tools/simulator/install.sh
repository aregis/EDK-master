if [ "$EUID" -ne 0 ]
  then echo "Please run as root"
  exit
fi
hash git 2>/dev/null || { echo >&2 "I require git but it's not installed.  Aborting."; exit 1; }
hash node 2>/dev/null || { echo >&2 "I require node but it's not installed.  Aborting."; exit 1; }

npm install


