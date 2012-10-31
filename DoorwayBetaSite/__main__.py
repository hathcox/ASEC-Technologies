# -*- coding: utf-8 -*-
"""

 Copyright [2012] [Redacted Labs]

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

-------
"""

from sys import argv
from datetime import datetime
from handlers import start_game

current_time = lambda: str(datetime.now()).split(' ')[1].split('.')[0]

def serve():
    """
    serves the application
    ----------------------
    """
    print "Now starting Doorway Beta Site!"
    start_game()

def create():
    ''' Creates/bootstraps the database '''
    from libs.ConfigManager import ConfigManager  # Sets up logging
    from models import __create__, boot_strap
    print('%s : Creating the database ... ' %
          current_time())
    __create__()
    if len(argv) == 3 and (argv[2] == 'bootstrap' or argv[2] == '-b'):
        print('\n\n\n' +
            '%s : Bootstrapping the database ... \n' % current_time())
        boot_strap()

options = ['serve', 'create']
if argv[1] in options:
    eval(argv[1])()