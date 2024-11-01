from datetime import datetime

from fastapi import UploadFile, File

from app.module_postgres import uuid_generator

"""_summary_
    Metadata of the file, when it's upload to the filesystem
    
    _fields_
    name - Name of the file
    size - Size of the file
    owner - Owner of the file and folder
    create - Date when file was create
    uuid - Name of the file in system
"""


class Metadata:
    name: str
    size: int
    owner_id: int
    create: datetime
    uuid: int

    """_summary_
        Initialization of the metadata class with validation
    """
    def __init__(self, owner_id: int, file: UploadFile = File(...)) -> None:
        if len(file.filename) > 30:
            raise ValueError("Имя файла не должно превышать 30 символов.")
        self.name = file.filename
        self.size = file.size
        self.owner_id = owner_id
        self.create = datetime.now()
        self.uuid = uuid_generator.generate_uuid(self.name, self.create)




