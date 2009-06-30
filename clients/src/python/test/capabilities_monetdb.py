#!/usr/bin/env python

# The contents of this file are subject to the MonetDB Public License
# Version 1.1 (the "License"); you may not use this file except in
# compliance with the License. You may obtain a copy of the License at
# http://monetdb.cwi.nl/Legal/MonetDBLicense-1.1.html
#
# Software distributed under the License is distributed on an "AS IS"
# basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
# License for the specific language governing rights and limitations
# under the License.
#
# The Original Code is the MonetDB Database System.
#
# The Initial Developer of the Original Code is CWI.
# Portions created by CWI are Copyright (C) 1997-July 2008 CWI.
# Copyright August 2008-2009 MonetDB B.V.
# All Rights Reserved.

import capabilities
import unittest
import warnings


#import logging
#logging.basicConfig(level=logging.DEBUG)
#logger = logging.getLogger('monetdb')


try:
    import monetdb.sql
except ImportError:
    import sys, os
    parent = os.path.join(sys.path[0], '..')
    sys.path.append(parent)
    import monetdb.sql


warnings.filterwarnings('error')

class Test_Monetdb_Sql(capabilities.DatabaseTest):
    MAPIPORT = 50000
    TSTDB = 'demo'
    import os
    if os.environ.has_key('MAPIPORT'):
        MAPIPORT = int(os.environ['MAPIPORT'])
    if os.environ.has_key('TSTDB'):
        TSTDB = os.environ['TSTDB']

    db_module = monetdb.sql
    connect_args = ()
    connect_kwargs = dict(database=TSTDB, port=MAPIPORT, autocommit=False)
    leak_test = False

    def test_DATETIME(self):
        from time import time
        ticks = time()
        def generator(row,col):
            return self.db_module.TimestampFromTicks(ticks+row*86400-col*1313)
        self.check_data_integrity(
                 ('col1 TIMESTAMP',),
                 generator)


    def test_TINYINT(self):
        # Number data
        def generator(row,col):
            v = (row*row) % 256
            if v > 127:
                v = v-256
            return v
        self.check_data_integrity(
            ('col1 TINYINT',),
            generator)

    def test_small_CHAR(self):
        # Character data
        def generator(row,col):
            i = (row*col+62)%256
            if i == 62: return ''
            if i == 63: return None
            return chr(i)
        self.check_data_integrity(
            ('col1 char(1)','col2 char(1)'),
            generator)

    def test_LONG(self):
        """ monetdb doesn't support LONG type """
        pass

    def test_LONG_BYTE(self):
        """ monetdb doesn't support LONG_BYTE """
        pass

if __name__ == '__main__':
    if Test_Monetdb_Sql.leak_test:
        import gc
        gc.enable()
        gc.set_debug(gc.DEBUG_LEAK)
    unittest.main()
