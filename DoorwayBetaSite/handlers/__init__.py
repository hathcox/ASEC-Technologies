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
import logging
import tornado.web
from tornado.web import Application
from tornado.web import StaticFileHandler 
from os import urandom, path
from base64 import b64encode
from handlers.BaseHandlers import *
from handlers.PublicHandlers import *
from handlers.UserHandlers import *
from modules.Menu import Menu
from modules.Recaptcha import Recaptcha
from models import dbsession
#Don't remove this comment, this is used as a pointhook to magically generate more handlers
#HANDLER_IMPORT_POINT_HOOK

logging.basicConfig(format = '[%(levelname)s] %(asctime)s - %(message)s', level = logging.DEBUG)

fileLogger = logging.FileHandler(filename = 'BlackBook.log')
fileLogger.setLevel(logging.DEBUG)
logging.getLogger('').addHandler(fileLogger)

application = Application([
        #Don't remove this comment, this is used as a pointhook to magically generate more handlers
        #HANDLER_APPLICATION_POINT_HOOK

        # Public handlers - Serves all public pages
        (r'/', WelcomeHandler),
        (r'/login', LoginHandler, {'dbsession': dbsession}),
        (r'/register',
            RegistrationHandler, {'dbsession': dbsession}),
        (r'/about', AboutHandler),
        (r'/logout', LogoutHandler),

        #User Handlers - Used for login / logout / settings
        (r'/user', HomeHandler, {'dbsession': dbsession}),
        (r'/download', DownloadHandler, {'dbsession': dbsession}),
        (r'/feedback', FeedbackHandler, {'dbsession': dbsession}),
        (r'/settings', SettingsHandler, {'dbsession': dbsession}),

        #Static Handlers - Serves static CSS, JavaScript and image files
        (r'/static/(.*)', StaticFileHandler, {'path': 'static'}),
      
        #Misc Handlers
        (r'/robots.txt', RobotsHandler),
        (r'/(.*)', NotFoundHandler)
],

    # Template directory
    template_path = 'templates',

    # Randomly generated secret key
    cookie_secret = b64encode(urandom(64)),

    #Register UI modules
    ui_modules={"Menu": Menu, "Recaptcha": Recaptcha},

    # Debug mode
    debug = True,

    # Enable XSRF forms (not optional)
    xsrf_cookies=True,
                  
    # Milli-Seconds between session clean up
    clean_up_timeout=int(60 * 1000),

    # Application version
    version = '0.0.1'
)

# Main entry point
def start_game():
     try:
          application.listen(8888)
          tornado.ioloop.IOLoop.instance().start()

     except KeyboardInterrupt:
          print "Shutting Down!"
