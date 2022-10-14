#include <HalonMTA.h>
#include <libxml/parser.h>
#include <libxml/relaxng.h>
#include <libxml/xmlerror.h>
#include <string>
#include <vector>

xmlRelaxNGParserCtxtPtr RNGparser = nullptr;
xmlRelaxNGPtr RNGschema = nullptr;

HALON_EXPORT
int Halon_version()
{
	return HALONMTA_PLUGIN_VERSION;
}

HALON_EXPORT
bool Halon_init(HalonInitContext* hic)
{
	HalonConfig *cfg;
	HalonMTA_init_getinfo(hic, HALONMTA_INIT_CONFIG, nullptr, 0, &cfg, nullptr);
	const char* schema = HalonMTA_config_string_get(HalonMTA_config_object_get(cfg, "schema"), nullptr);
	if (!schema)
		schema = "/storage/SVG_PS-latest.rng";

	RNGparser = xmlRelaxNGNewParserCtxt(schema);
	if (!RNGparser)
		return false;

	RNGschema = xmlRelaxNGParse(RNGparser);
	if (!RNGschema)
		return false;

	return true;
}

HALON_EXPORT
void Halon_cleanup()
{
	xmlRelaxNGFreeParserCtxt(RNGparser);
	xmlRelaxNGFree(RNGschema);
}

void XMLerrors(void *ctx, const char *msg, ...)
{
	char str[1024];
	va_list args;
	va_start(args, msg);
	vsnprintf(str, sizeof str, msg, args);
	va_end(args);
	((std::vector<std::string>*)ctx)->push_back(str);
}

void buildResponse(HalonHSLValue* ret, bool valid, const std::vector<std::string>& errors)
{
	HalonMTA_hsl_value_set(ret, HALONMTA_HSL_TYPE_ARRAY, nullptr, 0);

	HalonHSLValue *validk, *validv;
	HalonMTA_hsl_value_array_add(ret, &validk, &validv);
	HalonMTA_hsl_value_set(validk, HALONMTA_HSL_TYPE_STRING, "valid", 0);
	HalonMTA_hsl_value_set(validv, HALONMTA_HSL_TYPE_BOOLEAN, &valid, 0);

	if (!errors.empty())
	{
		HalonHSLValue *errorsk, *errorsv;
		HalonMTA_hsl_value_array_add(ret, &errorsk, &errorsv);
		HalonMTA_hsl_value_set(errorsk, HALONMTA_HSL_TYPE_STRING, "errors", 0);
		HalonMTA_hsl_value_set(errorsv, HALONMTA_HSL_TYPE_ARRAY, nullptr, 0);

		double i = 0.0;
		for (const auto & error : errors)
		{
			HalonHSLValue *errork, *errorv;
			HalonMTA_hsl_value_array_add(errorsv, &errork, &errorv);
			HalonMTA_hsl_value_set(errork, HALONMTA_HSL_TYPE_NUMBER, &i, 0);
			std::string trimmed = error.substr(0, error.find_last_not_of("\n") + 1);
			HalonMTA_hsl_value_set(errorv, HALONMTA_HSL_TYPE_STRING, trimmed.c_str(), trimmed.size());
			++i;
		}
	}
}

HALON_EXPORT
void bimi_svg_check(HalonHSLContext* hhc, HalonHSLArguments* args, HalonHSLValue* ret)
{
	HalonHSLValue* x = HalonMTA_hsl_argument_get(args, 0);
	char* svg = nullptr;
	size_t svglen;
	if (!x || HalonMTA_hsl_value_type(x) != HALONMTA_HSL_TYPE_STRING ||
			!HalonMTA_hsl_value_get(x, HALONMTA_HSL_TYPE_STRING, &svg, &svglen))
	{
		buildResponse(ret, false, { "Bad function argument" });
		return;
	}

	xmlParserCtxtPtr SVGctx = xmlNewParserCtxt();
	if (!SVGctx)
	{
		buildResponse(ret, false, { "xmlNewParserCtxt failed" });
		return;
	}
	xmlDocPtr SVGdoc = xmlCtxtReadMemory(SVGctx, svg, (int)svglen, nullptr, nullptr, XML_PARSE_NOERROR | XML_PARSE_NOWARNING);
	if (!SVGdoc)
	{
		xmlErrorPtr SVGErr = xmlCtxtGetLastError(SVGctx);
		if (SVGErr)
			buildResponse(ret, false, { SVGErr->message });
		else
			buildResponse(ret, false, { "xmlCtxtReadMemory failed" });
	}
	else
	{
		std::vector<std::string> errors;
		xmlRelaxNGValidCtxtPtr RNGvalidation = xmlRelaxNGNewValidCtxt(RNGschema);
		xmlRelaxNGSetValidErrors(RNGvalidation, XMLerrors, XMLerrors, &errors);
		buildResponse(ret, xmlRelaxNGValidateDoc(RNGvalidation, SVGdoc) == 0, errors);
		xmlRelaxNGFreeValidCtxt(RNGvalidation);
		xmlFreeDoc(SVGdoc);
	}
	xmlFreeParserCtxt(SVGctx);
}

HALON_EXPORT
bool Halon_hsl_register(HalonHSLRegisterContext* ptr)
{
	HalonMTA_hsl_module_register_function(ptr, "bimi_svg_check", &bimi_svg_check);
	return true;
}
