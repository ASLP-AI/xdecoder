# Created on 2018-07-04
# Author: Binbin Zhang

import MySQLdb

CONFIG = {
    'host': '127.0.0.1',
    'user': 'root',
    'passwd': 'admin',
    'db': 'asr',
    'charset': 'utf8'
}

class DbHelper:
    def __init__(self, **kvargs):
        self.connection = MySQLdb.connect(**kvargs)
        self.cursor = self.connection.cursor()

    def __del__(self):
        if self.connection is not None:
            self.connection.close()

    def asr_table_exist(self):
        sql_cmd = '''SELECT COUNT(*) FROM information_schema.tables WHERE table_name = 'history' '''
        self.cursor.execute(sql_cmd)
        if self.cursor.fetchone()[0] == 0:
            return False
        else:
            return True

    def create_asr_table(self):
        sql_cmd = '''CREATE TABLE history
                     (id int NOT NULL AUTO_INCREMENT,
                      time datetime NOT NULL,
                      wav_path text NOT NULL,
                      recognition text NOT NULL,
                      client_info text,
                      PRIMARY KEY (id) ) CHARSET=utf8'''
        self.cursor.execute(sql_cmd)

    def insert(self, wav_path, recognition, client_info):
        sql_cmd = '''INSERT INTO history (time, wav_path, recognition, client_info)
                     VALUES (Now(), "%s", "%s", "%s") ''' % (wav_path, recognition, client_info)
        try:
            self.cursor.execute(sql_cmd)
            self.connection.commit()
        except MySQLdb.Error as e:
            self.connection.rollback()

    def get_history_count(self):
        sql_cmd = '''SELECT COUNT(*) FROM history'''
        self.cursor.execute(sql_cmd)
        return self.cursor.fetchone()[0]

    def get_all_history(self, offset, number):
        sql_cmd = '''SELECT * FROM history ORDER BY id DESC LIMIT %d,%d''' % (offset, number)
        self.cursor.execute(sql_cmd)
        return self.cursor.fetchall()


if __name__ == '__main__':
    print(CONFIG)
    db = DbHelper(**CONFIG)
    if not db.asr_table_exist():
        db.create_asr_table()
    db.insert('a.wav', 'this is a dog', 'chrome')
    db.insert('b.wav', '你好', 'edge')
    print(db.get_all_history())

