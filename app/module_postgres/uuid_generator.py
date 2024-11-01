import datetime

"""_summary_
    
"""


def generate_uuid(name: str, create: datetime) -> int:
    return int(str(name.__hash__()) + str(int(create.timestamp())))
