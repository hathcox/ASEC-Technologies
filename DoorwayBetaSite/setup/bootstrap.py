# -*- coding: utf-8 -*-
'''
Created on Jul 2, 2012

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
'''


import os
import sys
import getpass

from libs.ConsoleColors import *
from libs.ConfigManager import ConfigManager
from models import dbsession, User, Permission

#Create Admin Account
user = User(
    user_name=unicode('admin'),
    approved=True
)
dbsession.add(user)
dbsession.flush()
user.password = 'nimda123'
dbsession.add(user)
dbsession.flush()
#Create Admin Permission
permission = Permission(
    permission_name="admin",
    user_id=user.id
)
dbsession.add(permission)
dbsession.flush()

#Create Default User Account

regular = User(
    user_name=unicode('user'),
    approved=True
    )
dbsession.add(regular)
dbsession.flush()
regular.password = '1234'
dbsession.add(regular)
dbsession.flush()