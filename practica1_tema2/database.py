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

    def execute(self, query, parameters=None) -> mariadb.Cursor:
        if parameters is None:
            parameters = ()
        # recorremos todas las conexiones para insertar
        lastcursor = None
        for conn in self.connections:
            cursor = conn.cursor()
            cursor.execute(query, parameters)
            conn.commit()
            lastcursor = cursor
        return lastcursor

    # para asegurar las tablas en destino
    def migrate(self):
        dropquery = "DROP TABLE IF EXISTS messages;"
        query = """
        CREATE TABLE IF NOT EXISTS messages(
            id INT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
            uuid VARCHAR(36) NOT NULL,
            client VARCHAR(255),
            message VARCHAR(255)
        );
        """
        self.execute(dropquery)
        self.execute(query)

    def deleteMessage(self, clave):
        query = """
        DELETE FROM messages WHERE uuid = ?;
        """
        self.execute(query, [clave])

    def getLastMessages(self)-> bytes:
        query = """
        SELECT uuid,client,message FROM messages LIMIT 14;
        """
        data = "\n **** Mensajes del servidor ***\n"

        cursor = self.execute(query)
        for row in cursor:
            print(row)
            data += f"|{row[0]}\t{row[1]}\t{row[2]}|\n"

        return data.encode('utf-8')