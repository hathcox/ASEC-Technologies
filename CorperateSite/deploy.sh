#!/bin/bash

echo "Coppying over files"
sudo cp -r /home/haddaway/Code/ASEC-Technologies/CorperateSite /var/www/

echo "Restarting Apache"
sudo service apache2 restart

echo "Finished!"