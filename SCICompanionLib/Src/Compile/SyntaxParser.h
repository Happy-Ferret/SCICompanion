#pragma once

#include "SCIStudioSyntaxParser.h"
#include "CPPSyntaxParser.h"

class SyntaxParser
{
public:
	bool Parse(sci::Script &script, CCrystalScriptStream &stream, ICompileLog *pLog = nullptr, bool fParseComments = false, SyntaxContext *pContext = nullptr);

private:
    SCISyntaxParser _sci;
    CPPSyntaxParser _cpp;
};

extern SyntaxParser g_Parser;