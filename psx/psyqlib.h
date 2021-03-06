#ifndef PSYQLIB_H
#define PSYQLIB_H

#include <unordered_map>
#include <functional>
#include <string>
#include <vector>
#include <deque>
#include <fstream>
#include <list>
#include <redasm/types/base_types.h>

#define PSYQ_MODULE_NAME_LENGTH 8
#define PSYQ_LIB_SIGNATURE_SIZE 3
#define PSYQ_LIB_SIGNATURE      "LIB"

//struct PSYQString { u8 length; char data[1]; };
struct PSYQFile { char signature[PSYQ_LIB_SIGNATURE_SIZE]; u8 version; };

enum PSYQState: u8
{
    SectionEOF      = 0,
    SectionCode     = 2,
    SectionSwitch   = 6,
    SectionBss      = 8,
    SectionPatch    = 10,
    SymbolDef       = 12,
    SymbolRef       = 14,
    SectionSymbol   = 16,
    Processor       = 46,
    SymbolBss       = 48,
};

enum PSYQPatchState: u8
{
    PatchToReference = 2,
    PatchToSection   = 4,
    PatchToValue     = 44,
    PatchToDiff      = 46,
};

struct PSYQBss
{
    u16 symbolnumber, sectionnumber;
    u32 size;
    std::string name;
};

struct PSYQReference
{
    u16 symbolnumber;
    std::string name;
};

struct PSYQDefinition
{
    u16 symbolnumber, sectionnumber;
    u32 offset;
    std::string name;
};

enum PSYQPatchType: u8
{
    Jump = 74,
    Bss1 = 82,
    Bss2 = 84,
};

struct PSYQPatch
{
    PSYQPatch(): type(0), offset(0), patchsection(0), symbolnumber(0), sectionnumber(0) { patchvalue = {0, 0}; }

    u8 type;
    u16 offset;
    u16 patchsection; // It's not part of the format

    struct {
        u8 unk1;
        u32 value;
    } patchvalue;

    struct {
        u16 symbolnumber;
        u16 sectionnumber;
    };
};

struct PSYQSection
{
    PSYQSection(): sizebss(0) { }

    u16 symbolnumber, group;
    u8 alignment;
    std::string name;
    std::vector<u8> code;
    u32 sizebss;
};

struct PSYQLink
{
    char signature[3]; // "LNK"
    u8 version, processorType;
    std::unordered_map<u16, PSYQSection> sections;
    std::unordered_map<u16, PSYQDefinition> definitions;
    std::unordered_map<u16, PSYQReference> references;
    std::unordered_map<u16, PSYQBss> bss;
    std::list<PSYQPatch> patches;
};

struct PSYQModule
{
    std::string name;
    u32 win_datetime; // https://docs.microsoft.com/en-us/cpp/c-runtime-library/32-bit-windows-time-date-formats
    u32 offsetlink, offsetnext;
    std::list<std::string> names;
    PSYQLink link;
};

class PSYQLib
{
    private:
        typedef std::function<bool()> StateCallback;

    public:
        PSYQLib(const std::string& infile);
        const std::list<PSYQModule>& modules() const;
        bool load();

    private:
        bool executeStateSectionEof();
        bool executeStateSectionCode();
        bool executeStateSectionSwitch();
        bool executeStateSectionBss();
        bool executeStateSectionPatch();
        bool executeStateSymbolRef();
        bool executeStateSymbolDef();
        bool executeStateSectionSymbol();
        bool executeStateProcessor();
        bool executeStateSymbolBss();

    private:
        bool executeStatePatchToReference();
        bool executeStatePatchToSection();
        bool executeStatePatchToValue();

    private:
        std::list<std::string> readStringTable();
        time_t timestamp(u32 wdatetime) const;
        std::string readString();
        PSYQModule readModule();
        PSYQState readState();
        bool readAllModules();
        bool readLink();

    private:
        std::ifstream m_instream;
        std::unordered_map<PSYQState, StateCallback> m_statehandler;
        std::unordered_map<PSYQPatchState, StateCallback> m_patchhandler;
        std::list<PSYQModule> m_modules;
        PSYQFile m_header;
        PSYQModule m_currentmodule;
        PSYQLink* m_currentlink;
        PSYQSection* m_currentsection;
};

#endif // PSYQLIB_H
