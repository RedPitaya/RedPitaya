#include "new_main.h"

#include <DataManager.h>
#include <CustomParameters.h>

/*std::string GenerateFloatArrays()
{
	JSONNode n(JSON_NODE);
	
	JSONNode c1(JSON_ARRAY);
	JSONNode c2(JSON_ARRAY);
	c1.set_name("ArrayOfFloats1");
	c2.set_name("ArrayOfFloats2");
	float start_agl = (-1)*M_PI/2;	
	int array_size = 2048;	
	for(int i=0; i < array_size; i++)
	{	
		float agl = start_agl + M_PI * (float)i / (float)array_size;		
		float res = sin(agl);
		c1.push_back(JSONNode("", res));
		c2.push_back(JSONNode("", res));
	}
	n.push_back(c1);
	n.push_back(c2);	
	std::string jc = n.write_formatted();

 	return jc;
}*/

CFloatSignal Signal1("signal1", 2048, 0.f);
CFloatSignal Signal2("signal2", 2048, 0.f);
CFloatSignal Signal3("signal3", 2048, 0.f);
CFloatSignal Signal4("signal4", 2048, 0.f);
CFloatSignal Signal5("signal5", 2048, 0.f);

CIntParameter MyParameter00("param00", CBaseParameter::RW, 22, 1, 20, 50);
CIntParameter MyParameter01("param01", CBaseParameter::RW, 22, 1, 20, 50);
CIntParameter MyParameter02("param02", CBaseParameter::RW, 22, 1, 20, 50);
CIntParameter MyParameter03("param03", CBaseParameter::RW, 22, 1, 20, 50);
CIntParameter MyParameter04("param04", CBaseParameter::RW, 22, 1, 20, 50);
CIntParameter MyParameter05("param05", CBaseParameter::RW, 22, 1, 20, 50);
CIntParameter MyParameter06("param06", CBaseParameter::RW, 22, 1, 20, 50);
CIntParameter MyParameter07("param07", CBaseParameter::RW, 22, 1, 20, 50);
CIntParameter MyParameter08("param08", CBaseParameter::RW, 22, 1, 20, 50);
CIntParameter MyParameter09("param09", CBaseParameter::RW, 22, 1, 20, 50);
CIntParameter MyParameter10("param10", CBaseParameter::RW, 22, 1, 20, 50);
CIntParameter MyParameter11("param11", CBaseParameter::RW, 22, 1, 20, 50);
CIntParameter MyParameter12("param12", CBaseParameter::RW, 22, 1, 20, 50);
CIntParameter MyParameter13("param13", CBaseParameter::RW, 22, 1, 20, 50);
CIntParameter MyParameter14("param14", CBaseParameter::RW, 22, 1, 20, 50);
CIntParameter MyParameter15("param15", CBaseParameter::RW, 22, 1, 20, 50);
CIntParameter MyParameter16("param16", CBaseParameter::RW, 22, 1, 20, 50);
CIntParameter MyParameter17("param17", CBaseParameter::RW, 22, 1, 20, 50);
CIntParameter MyParameter18("param18", CBaseParameter::RW, 22, 1, 20, 50);
CIntParameter MyParameter19("param19", CBaseParameter::RW, 22, 1, 20, 50);
CIntParameter MyParameter20("param20", CBaseParameter::RW, 22, 1, 20, 50);
CIntParameter MyParameter21("param21", CBaseParameter::RW, 22, 1, 20, 50);
CIntParameter MyParameter22("param22", CBaseParameter::RW, 22, 1, 20, 50);
CIntParameter MyParameter23("param23", CBaseParameter::RW, 22, 1, 20, 50);
CIntParameter MyParameter24("param24", CBaseParameter::RW, 22, 1, 20, 50);
CIntParameter MyParameter25("param25", CBaseParameter::RW, 22, 1, 20, 50);
CIntParameter MyParameter26("param26", CBaseParameter::RW, 22, 1, 20, 50);
CIntParameter MyParameter27("param27", CBaseParameter::RW, 22, 1, 20, 50);
CIntParameter MyParameter28("param28", CBaseParameter::RW, 22, 1, 20, 50);
CIntParameter MyParameter29("param29", CBaseParameter::RW, 22, 1, 20, 50);
CIntParameter MyParameter30("param30", CBaseParameter::RW, 22, 1, 20, 50);
CIntParameter MyParameter31("param31", CBaseParameter::RW, 22, 1, 20, 50);
CIntParameter MyParameter32("param32", CBaseParameter::RW, 22, 1, 20, 50);
CIntParameter MyParameter33("param33", CBaseParameter::RW, 22, 1, 20, 50);
CIntParameter MyParameter34("param34", CBaseParameter::RW, 22, 1, 20, 50);
CIntParameter MyParameter35("param35", CBaseParameter::RW, 22, 1, 20, 50);
CIntParameter MyParameter36("param36", CBaseParameter::RW, 22, 1, 20, 50);
CIntParameter MyParameter37("param37", CBaseParameter::RW, 22, 1, 20, 50);
CIntParameter MyParameter38("param38", CBaseParameter::RW, 22, 1, 20, 50);
CIntParameter MyParameter39("param39", CBaseParameter::RW, 22, 1, 20, 50);

CFloatParameter MyParameter40("param40", CBaseParameter::RW, 2.3, 1, 2.0, 5.0);
CFloatParameter MyParameter41("param41", CBaseParameter::RW, 2.3, 1, 2.0, 5.0);
CFloatParameter MyParameter42("param42", CBaseParameter::RW, 2.3, 1, 2.0, 5.0);
CFloatParameter MyParameter43("param43", CBaseParameter::RW, 2.3, 1, 2.0, 5.0);
CFloatParameter MyParameter44("param44", CBaseParameter::RW, 2.3, 1, 2.0, 5.0);
CFloatParameter MyParameter45("param45", CBaseParameter::RW, 2.3, 1, 2.0, 5.0);
CFloatParameter MyParameter46("param46", CBaseParameter::RW, 2.3, 1, 2.0, 5.0);
CFloatParameter MyParameter47("param47", CBaseParameter::RW, 2.3, 1, 2.0, 5.0);
CFloatParameter MyParameter48("param48", CBaseParameter::RW, 2.3, 1, 2.0, 5.0);
CFloatParameter MyParameter49("param49", CBaseParameter::RW, 2.3, 1, 2.0, 5.0);
CFloatParameter MyParameter50("param50", CBaseParameter::RW, 2.3, 1, 2.0, 5.0);
CFloatParameter MyParameter51("param51", CBaseParameter::RW, 2.3, 1, 2.0, 5.0);
CFloatParameter MyParameter52("param52", CBaseParameter::RW, 2.3, 1, 2.0, 5.0);
CFloatParameter MyParameter53("param53", CBaseParameter::RW, 2.3, 1, 2.0, 5.0);
CFloatParameter MyParameter54("param54", CBaseParameter::RW, 2.3, 1, 2.0, 5.0);
CFloatParameter MyParameter55("param55", CBaseParameter::RW, 2.3, 1, 2.0, 5.0);
CFloatParameter MyParameter56("param56", CBaseParameter::RW, 2.3, 1, 2.0, 5.0);
CFloatParameter MyParameter57("param57", CBaseParameter::RW, 2.3, 1, 2.0, 5.0);
CFloatParameter MyParameter58("param58", CBaseParameter::RW, 2.3, 1, 2.0, 5.0);
CFloatParameter MyParameter59("param59", CBaseParameter::RW, 2.3, 1, 2.0, 5.0);
CFloatParameter MyParameter60("param60", CBaseParameter::RW, 2.3, 1, 2.0, 5.0);
CFloatParameter MyParameter61("param61", CBaseParameter::RW, 2.3, 1, 2.0, 5.0);
CFloatParameter MyParameter62("param62", CBaseParameter::RW, 2.3, 1, 2.0, 5.0);
CFloatParameter MyParameter63("param63", CBaseParameter::RW, 2.3, 1, 2.0, 5.0);
CFloatParameter MyParameter64("param64", CBaseParameter::RW, 2.3, 1, 2.0, 5.0);
CFloatParameter MyParameter65("param65", CBaseParameter::RW, 2.3, 1, 2.0, 5.0);
CFloatParameter MyParameter66("param66", CBaseParameter::RW, 2.3, 1, 2.0, 5.0);
CFloatParameter MyParameter67("param67", CBaseParameter::RW, 2.3, 1, 2.0, 5.0);
CFloatParameter MyParameter68("param68", CBaseParameter::RW, 2.3, 1, 2.0, 5.0);
CFloatParameter MyParameter69("param69", CBaseParameter::RW, 2.3, 1, 2.0, 5.0);
CFloatParameter MyParameter70("param70", CBaseParameter::RW, 2.3, 1, 2.0, 5.0);
CFloatParameter MyParameter71("param71", CBaseParameter::RW, 2.3, 1, 2.0, 5.0);
CFloatParameter MyParameter72("param72", CBaseParameter::RW, 2.3, 1, 2.0, 5.0);
CFloatParameter MyParameter73("param73", CBaseParameter::RW, 2.3, 1, 2.0, 5.0);
CFloatParameter MyParameter74("param74", CBaseParameter::RW, 2.3, 1, 2.0, 5.0);
CFloatParameter MyParameter75("param75", CBaseParameter::RW, 2.3, 1, 2.0, 5.0);
CFloatParameter MyParameter76("param76", CBaseParameter::RW, 2.3, 1, 2.0, 5.0);
CFloatParameter MyParameter77("param77", CBaseParameter::RW, 2.3, 1, 2.0, 5.0);
CFloatParameter MyParameter78("param78", CBaseParameter::RW, 2.3, 1, 2.0, 5.0);
CFloatParameter MyParameter79("param79", CBaseParameter::RW, 2.3, 1, 2.0, 5.0);

CBooleanParameter MyParameter80("param80", CBaseParameter::RW, false, 1);
CBooleanParameter MyParameter81("param81", CBaseParameter::RW, false, 1);
CBooleanParameter MyParameter82("param82", CBaseParameter::RW, false, 1);
CBooleanParameter MyParameter83("param83", CBaseParameter::RW, false, 1);
CBooleanParameter MyParameter84("param84", CBaseParameter::RW, false, 1);
CBooleanParameter MyParameter85("param85", CBaseParameter::RW, false, 1);
CBooleanParameter MyParameter86("param86", CBaseParameter::RW, false, 1);
CBooleanParameter MyParameter87("param87", CBaseParameter::RW, false, 1);
CBooleanParameter MyParameter88("param88", CBaseParameter::RW, false, 1);
CBooleanParameter MyParameter89("param89", CBaseParameter::RW, false, 1);
CBooleanParameter MyParameter90("param90", CBaseParameter::RW, false, 1);
CBooleanParameter MyParameter91("param91", CBaseParameter::RW, false, 1);
CBooleanParameter MyParameter92("param92", CBaseParameter::RW, false, 1);
CBooleanParameter MyParameter93("param93", CBaseParameter::RW, false, 1);
CBooleanParameter MyParameter94("param94", CBaseParameter::RW, false, 1);
CBooleanParameter MyParameter95("param95", CBaseParameter::RW, false, 1);
CBooleanParameter MyParameter96("param96", CBaseParameter::RW, false, 1);
CBooleanParameter MyParameter97("param97", CBaseParameter::RW, false, 1);
CBooleanParameter MyParameter98("param98", CBaseParameter::RW, false, 1);
CBooleanParameter MyParameter99("param99", CBaseParameter::RW, false, 1);

void UpdateParams(void)
{
	// do something
}

void UpdateSignals(void)
{
	// do something
}

void OnNewParams(void)
{
	// do something
	//CDataManager::GetInstance()->UpdateAllParams();
}

