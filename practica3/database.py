import mariadb

"""
DatabasePool maneja todas las conexion a base datos
"""
class DatabasePool:
    def __init__(self, credentials):

        self.connections = []

        for credential in credentials:
            self.createPool(
                user= credential["user"],
                pwd= credential["pwd"],
                host= credential["host"],
                port= credential["port"] ,
                dbname= credential["dbname"]
            )


    def createPool(self,host,user,pwd,dbname, port = 3306):
        try:
            conn = mariadb.connect(
                user=user,
                password=pwd,
                host=host,
                port=port,
                database=dbname
            )
            self.connections.append(conn)
            cursor = conn.cursor()
            cursor.execute("SELECT 1")
            conn.commit()
        except mariadb.Error as e:
            print(e)

    def execute(self, query, parameters=None):
        if parameters is None:
            parameters = ()
        # recorremos todas las conexiones para insertar
        for conn in self.connections:
            cursor = conn.cursor()
            cursor.execute(query, parameters)
            conn.commit()

    # para asegurar las tablas en destino
    def migrate(self):
        query = """CREATE TABLE IF NOT EXISTS messages(
            id INT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
            name VARCHAR(255),
            message VARCHAR(255)
        );"""
        self.execute(query)

