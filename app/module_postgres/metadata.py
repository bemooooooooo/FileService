from fastapi import UploadFile, File

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
    owner: str
    create: int
    uuid: int
    
    """_summary_
        Initialization of the metadata class with validation
    """    
    def __init__(self: UploadFile = File(...)) -> None:
        pass
