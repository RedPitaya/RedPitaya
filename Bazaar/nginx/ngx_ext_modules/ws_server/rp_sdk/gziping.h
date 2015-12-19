#pragma once
#include <string>
#include "gzip.h"

using namespace CryptoPP;

void Gziping(const std::string& in, std::string& out)
{
	/*
	Gzip zipper(new StringSink(out));
	zipper.Put((byte*)in.data(), in.size());
	zipper.MessageEnd();
	*/

	StringSource ss(in, true, new Gzip(new StringSink(out)));
}
