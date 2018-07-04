# Created on 2018-07-04
# Author: Binbin Zhang

import MySQLdb


class DbHelper:
    def __init__(self):
        self.connection = MySQLdb.connect(host='127.0.0.1', user='root', passwd='admin', db='asr', charset='utf8')
        self.cursor = self.connection.cursor()

    def __del__(self):
        if self.connection is not None:
            self.connection.close()
    
    def create_table(self):
        sql_cmd = '''SELECT COUNT(*) FROM information_schema.tables WHERE table_name = 'history' '''
        self.cursor.execute(sql_cmd)
        # if table doesn't exist, create one
        if self.cursor.fetchone()[0] == 0:
            sql_cmd = '''CREATE TABLE history 
                         (id int NOT NULL AUTO_INCREMENT,
                          time datetime NOT NULL,
                          wav_path text NOT NULL,
                          recognition text NOT NULL,
                          client_info text,
                          PRIMARY KEY (id) )'''
            self.cursor.execute(sql_cmd)

    def insert(self, wav_path, recognition, client_info):
        sql_cmd = '''INSERT INTO history (time, wav_path, recognition, client_info)
                     VALUES (Now(), "%s", "%s", "%s") ''' % (wav_path, recognition, client_info)
        print(sql_cmd)
        try:
            self.cursor.execute(sql_cmd)
            self.connection.commit()
        except MySQLdb.Error as e:
            self.connection.rollback()

    def get_all_history(self):
        sql_cmd = '''SELECT * FROM history ORDER BY id DESC'''
        self.cursor.execute(sql_cmd)
        return self.cursor.fetchall()


if __name__ == '__main__':
    db = DbHelper()
    db.create_table()
    db.insert('a.wav', 'this is a dog', 'chrome')
    db.insert('b.wav', 'this is a cat', 'edge')
    print(db.get_all_history())

