using System;
using System.Collections.Generic;


[Serializable]
public class SaveEntry
{
    public string id;
    public string jsonData;   // serialized object
    public string type; // the object's data type
}

[Serializable]
public class JsonWrapper
{
    public List<SaveEntry> entries = new List<SaveEntry>();
}

