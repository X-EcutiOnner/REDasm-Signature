#ifndef PSYQLIB_GENERATOR_H
#define PSYQLIB_GENERATOR_H

#include "../patterngenerator.h"

class PsyQLibGenerator: public PatternGenerator
{
    public:
        PsyQLibGenerator();
        virtual std::string name() const;
        virtual std::string assembler() const;
        virtual bool test(const std::string& infile);
        virtual void generate(const std::string& infile);

    private:
        void stopAtDelaySlot(std::string& subpattern) const;
        void fixTail(std::string& subpattern) const;
};

#endif // PSYQLIB_GENERATOR_H
