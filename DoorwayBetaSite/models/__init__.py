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
   

this sets up sqlalchemy.
for more information about sqlalchemy check out <a href="http://www.sqlalchemy.org/">www.sqlalchemy.org</a>
"""

from sqlalchemy import create_engine
from sqlalchemy import Column, ForeignKey, Table
from sqlalchemy.types import Integer
from sqlalchemy.orm import sessionmaker
from models.BaseObject import BaseObject
from libs.ConfigManager import *

metadata = BaseObject.metadata

# set the connection string here
config = ConfigManager.Instance()
db_connection = 'mysql://%s:%s@%s/%s' % (
    config.db_user, config.db_password, config.db_server, config.db_name)
engine = create_engine(db_connection)
Session = sessionmaker(bind=engine, autocommit=True)

#User <-> User join table for friends
association_table = Table('user_friend', BaseObject.metadata,
                          Column('user_id',
                                 Integer, ForeignKey('user.id'), nullable=False),
                          Column('friend_id',
                                 Integer, ForeignKey('user.id'), nullable=False)
                          )

# import the dbsession instance to execute queries on your database
dbsession = Session(autoflush = True)

# import models.
from models.BaseObject import BaseObject
from models.User import User
from models.Post import Post
from models.Comment import Comment
from models.Permission import Permission
from models.Tag import Tag

# calling this will create the tables at the database
__create__ = lambda: (setattr(engine, 'echo', True), metadata.create_all(engine))

# Bootstrap the databases
def boot_strap():
    import setup.bootstrap