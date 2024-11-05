from module_postgres.database_controller import DataBase
from fastapi import FastAPI
from fastapi import UploadFile


app = FastAPI()
db = DataBase()


@app.get("/file/get/{name}", response_model=str)
async def getFile(name: str) -> str:
    """Get the file from filesystem by name.
    Args:
        name (str): name of the file.

    Returns:
        str: url to file on the vps.
    """    """"""
    return db.get(name)

@app.post("/file/post/", response_model=UploadFile)
async def postFile(owner_id: int, file: UploadFile) -> None:
    """Post the file to the filesystem

    Args:
        owner_id (int): id of the Auth user.
        file (UploadFile): file from user.
    """    
    # Добавить валидацию данных
    return db.post(owner_id, file)