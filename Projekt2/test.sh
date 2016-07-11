# -*- coding: utf-8 -*-

# Port
PORT=4001
# KLIENT
KLIENT=./client/client 
# SERVER
SERVER=./server/server
# HOST
HOST=localhost

# Vycistenie po predchadzajucich chodoch testov
make clean
# Aktivacia makefile
make

# Testy na chovanie klienta pri zlych argumentoch

# Test - nezadany port
printf "\nTest - test nezadany port\n"
$KLIENT -h $HOST -u temp

# Test - nezadany host
printf "\nTest - test nezadany host\n"
$KLIENT -p $PORT -u temp

# Test - neexistujuci port
printf "\nTest - test neexistujuci port\n"
$KLIENT -h $HOST  p 800000-u temp

# Test - zadane aj -u aj -d 
printf "\nTest - zadany aj -u aj -d\n"
$KLIENT -p $PORT -h $HOST -u temp -d temp

# Test - pripojenie na vypnuty server
printf "\nTest - test pripojenia na vypnuty server\n"
$KLIENT -p $PORT -h $HOST -u temp

# START SERVERA
printf "\n\nSTARTUJEM SERVER\n\n"
$SERVER -p $PORT & SERVER_PID=$! 2>server_error >server_stdout 

# Test - neexitujuci subor
printf "\nTest - upload neexistujuceho suboru\n"
$KLIENT -p $PORT -h $HOST -u temp




# Testy na fungovanie komunikacie

# Test - neexitujuci subor
printf "\nTest - download neexistujuceho suboru\n"
$KLIENT -p $PORT -h $HOST -d temp

# Test - download textoveho suboru
printf "\nTest - download suboru, textac\n"
$KLIENT -p $PORT -h $HOST -d server_text
sleep 2
diff -s ./server/server_text ./client/server_text

# Test - upload textoveho suboru
printf "\nTest - upload suboru, textac\n"
$KLIENT -p $PORT -h $HOST -u client_text
sleep 2
diff -s ./server/client_text ./client/client_text

# Test - download obrazku
printf "\nTest - download suboru, obrazok\n"
$KLIENT -p $PORT -h $HOST -d server_image.jpg
sleep 2
diff -s ./server/server_image.jpg ./client/server_image.jpg

# Test - upload ubrazku
printf  "\nTest - upload suboru, obrazok\n"
$KLIENT -p $PORT -h $HOST -u client_image.jpg
sleep 2
diff -s ./server/client_image.jpg ./client/client_image.jpg


# Ukoncenie servera a pripadnych sirot
killall -q -e server &