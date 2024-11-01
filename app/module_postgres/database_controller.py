import psycopg2 as pg
from dotenv import load_dotenv
from metadata import Metadata
import os

"""_summary_
    __init__ - connect to database
    
    _fields_
    connection - connection pool to postgres with args from .env
    cursor - cursor on pool
"""
class DataBase:
    """_summary_
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
            
    """_summary_
        post(metadata: Metadata) - post into database metadata of the file, Metadata - class in metadata.py
    """     
    def post(self, metadata: Metadata) -> int:
        try:
            self.cursor.execute(''''INSERT INTO file (owner_id, file_name, file_size, uuid, create_at) VALUES (%s, %s, %s, %s, %s)''', 
                            (metadata.owner, metadata.name, metadata.size, metadata.uuid, metadata.create))
        except:
            print("Cannot insert metadata")
            return 503
        return 200
        
        
    """_summary_
        get(name: str) - get url to file with name
    """    
    def get(name: str) -> str:
        pass