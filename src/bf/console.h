#pragma once

// no idea what these are, see ConsoleObject::getObjectRange
struct ConsoleObjectRange {
    int min, max;
};
struct ConsoleObjectRangeString {
    bfs::string min, max;
};

// base class for console objects (console commands)
class ConsoleObject {
public:
    // 1: read/write, 0: read only, 2: write only
    int type;
    const char* objectname; // name of object (before dot)
    const char* functionname; // name of function (after dot)
    int access; // 1: can always access it TODO: figure out higher, more restricted levels
    int minargcount; // minimum number of parameters
    int maxargcount; // maximum number of parameters

    // variables used when executing commands (see ConsoleObjectBaseImpl::execute)
    int argcount; // set to number of parameters passed to command
    intptr_t args[11]; // parameters passed to command

    // return value description
    const char* retdesc; // return value type
    const char* argdesc[10]; // parameter types
    int argtype[10]; // 0 for strings, -1 otherwise, if -1 setArgFromString is called to convert parameter, 0 is probably broken?

    bool hasreturnvalue; // updated when executing, if set to 1 then 
    bool isdynamic;
    int8_t returnbyte; // container for return value if it is a byte

    virtual ~ConsoleObject() {};
    virtual void* getObject() const { return 0; }; // unused?
    virtual const char* getObjectName() const { return objectname; };
    virtual const char* getFunctionName() const { return functionname; };
    virtual int getType() const { return type; };
    virtual int getAccess() const { return access; };
    virtual const char* getReturnType() const { return retdesc; };
    virtual const char* getArgType(int arg) const { return argdesc[arg]; };
    virtual int getMinNoArgs() const { return minargcount; };
    virtual int getMaxNoArgs() const { return maxargcount; };
    virtual bool isActive() const { return true; };
    virtual bool hasRange() const { return false; };
    virtual int getRange() const { return 0; }; 
    virtual bfs::string getRangeAsString() const { return ""; };
    virtual bool hasReturnValue() const { return hasreturnvalue; }; // if there is a return value after execute
    virtual bool isDynamic() const { return isdynamic; }; // mostly returns true, unused?
    virtual bfs::string execute(bfs::string* argv, int argc); // sets argcount, args, calls checkObjectRange, setArgFromString, executeObjectMethod, getReturnValueAsString
    virtual bool isObjectActive() const { return false; }; // can the command be executed? check if underlying object exists
    virtual void* executeObjectMethod() { return 0; }; // uses argcount and args to execute the effect of the command, returns pointer to return value but it is unused
    virtual bfs::string getReturnValueAsString() { return ""; }; // converts return value to string, should check hasreturnvalue
    virtual void setArgFromString(int arg, bfs::string const& value) {}; // called for each command parameter to convert it, result is stored in args[]
    virtual bool objectHasRange() const { return false; }; // unused?
    virtual ConsoleObjectRange getObjectRange() const { return { 0, 0 }; }; // unused?
    virtual ConsoleObjectRangeString getObjectRangeAsString() const { return { "", ""}; }; // unused?
    virtual bool checkObjectRange(bool) { return false; }; // used in execute(), but no effect?

};

// class that holds all registered console objects
class ConsoleObjects {
public:
    Hash <bfs::string, bfs::string> typeconversions;
    bfs::map<bfs::string, ConsoleObject*> nameToObject;
    bfs::list<ConsoleObject*> objects;
    static ConsoleObjects* getSingleton() { return reinterpret_cast<ConsoleObjects*>(0x009AB610); };
    bool registerConsoleObjects(bfs::list<ConsoleObject*>& list);
};


void register_custom_console_commands();
