#include "generators.h"
#include <iostream>

#define WRAP_TO_STRING(...)   #__VA_ARGS__
#define GENERATOR(generator)  WRAP_TO_STRING(../generators/generator##_generator.h)

#define REGISTER_GENERATOR(T) Generators::registered.push_back([&](const std::string& infile, const std::string& prefix) -> PatternGenerator* { \
                                 return Generators::generateCallback<T##Generator>(infile, prefix); \
                              })

/* *** Generators *** */
#include GENERATOR(psyqlib)
#include GENERATOR(json)

std::list<GeneratorCallback> Generators::registered;
std::list< std::unique_ptr<PatternGenerator> > Generators::active;

void Generators::init()
{
    if(!Generators::registered.empty())
        return;

    REGISTER_GENERATOR(PsyQLib);
    REGISTER_GENERATOR(JSON);
}

PatternGenerator *Generators::getPattern(const std::string &infile, const std::string &prefix, bool verbose)
{
    for(auto it = active.begin(); it != active.end(); it++)
    {
        if(!(*it)->generate(infile, prefix))
            continue;

        if(verbose)
            std::cout << (*it)->name() << ": " << infile << std::endl;

        return it->get();
    }

    for(auto it = registered.begin(); it != registered.end(); it++)
    {
        PatternGenerator* patterngenerator = (*it)(infile, prefix);

        if(!patterngenerator)
            continue;

        active.emplace_back(patterngenerator);
        return active.back().get();
    }

    return NULL;
}

bool Generators::saveAsSDB(REDasm::SignatureDB &sigdb)
{
    for(auto it = active.begin(); it != active.end(); it++)
    {
        if((*it)->saveAsSDB(sigdb))
            continue;

        std::cout << "ERROR: Cannot save SDB pattern(s)" << std::endl;
        return false;
    }

    return true;
}

bool Generators::saveAsJSON(json &patterns)
{
    for(auto it = active.begin(); it != active.end(); it++)
    {
        if((*it)->saveAsJSON(patterns))
            continue;

        std::cout << "ERROR: Cannot save JSON pattern(s)" << std::endl;
        return false;
    }

    return true;
}