import psycopg2 as pg
from dotenv import load_dotenv
from metadata import Metadata
import os

class DataBase:
    """## Class adapter to PostgreSQL
    #### Methods:
        - init
        - post
        - get
    #### Fields:
        - connection - connection pool to postgres with args from .env
        - cursor - cursor on pool
    """
    
    def __init__(self) -> None:
        load_dotenv()
        try:
            self.connection = pg.connect(host=os.getenv("DATABASE_HOST"),
                                    database=os.getenv("DATABASE_NAME"),
                                    port=os.getenv("DATABASE_PORT"),
                                    user=os.getenv("DATABASE_USER"),
                                    password=os.getenv("DATABASE_PASSWORD"))
            self.cursor = self.connection.cursor()
            self.connection.autocommit = True
        except Exception as error:
            print("Something went wrong")
            print(error)
            
    def post(self, metadata: Metadata) -> int:
        """
        ### Post into database metadata of the file, Metadata - class in metadata.py
        """     
        try:
            self.cursor.execute(''''INSERT INTO file (owner_id, file_name, file_size, uuid, create_at) VALUES (%s, %s, %s, %s, %s)''', 
                            (metadata.owner, metadata.name, metadata.size, metadata.uuid, metadata.create))
            return 200
        except:
            print("Cannot insert metadata")
            return 503
        
        
    def get(self, name: str) -> str | int:
        """
            ### Get url to file with name
        """    
        try:
            self.cursor.execute('''SELECT uuid FROM file WHERE file_name = %s''', (name))
            data = self.cursor.fetchall()
            return data
        except:
            print("Cannot get metadata")
            return 503
        