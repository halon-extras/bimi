# Brand Indicators for Message Identification

This plugin adds BIMI validation. It supports both VMC/CMC (see current ca.cert) and "externally verified" certificates/domains. It can also validates SVG image profiles as required by the BIMI specification. This plugin depends on libxml2 and its validation is based on a RelaxNG scheme.

## Installation

Follow the [instructions](https://docs.halon.io/manual/comp_install.html#installation) in our manual to add our package repository and then run the below command.

### Ubuntu

```
apt-get install halon-extras-bimi
```

### RHEL

```
yum install halon-extras-bimi
```

## Usage

```
import { dmarc } from "extras://dmarc";
import { bimi, bimi_vmc, bimi_svg_check } from "extras://bimi";

$mail = $arguments["mail"];
$dmarc = dmarc($mail, $connection["remoteip"], $connection["helo"]["host"] ?? "", $transaction["senderaddress"]["domain"] ?? "");
$bimi = bimi($mail, $dmarc);

// Verified Mark Certificate (VMC)
if ($bimi["record"]["a"])
{
	$bimi_vmc = bimi_vmc($bimi);
	if ($bimi_vmc["indicator"])
	{
		$bimi_svg = bimi_svg_check($bimi_vmc["indicator"]);
		if ($bimi_svg["valid"])
		{
			/*
			// to get the markType, extract the following OID from the subject of the BIMI Evidence Document
			$markType = array_find(function ($v) { return $v[0] == "1.3.6.1.4.1.53087.1.13"; }, $bimi_vmc["subject"] ?? []);
			*/

			$mail->addHeader("BIMI-Indicator",
				str_strip(array_join(pcre_match_all(#/(^(.{0,49})|(.{0,64}))/, base64_encode($bimi_vmc["indicator"]))[1], "\r\n ")),
				["encode" => false]);
			// The l= location (indicator-uri) is not validated so as per "Construct BIMI-Location URI"
			// we do not include l=".$bimi["record"]["l"], make note of that if using the policy.indicator-uri
			// verify the logo with the policy.indicator-hash before rendering.
			$mail->addHeader("BIMI-Location",
				"v=BIMI1; a=".$bimi["record"]["a"]);
			$mail->addHeader("BIMI-Logo-Preference",
				"avp=".($bimi["record"]["avp"] == "personal" ? "personal" : "brand"));
			$mail->addHeader("Authentication-Results",
				gethostname()."; bimi=pass header.d=".$bimi["domain"]." header.selector=".$bimi["selector"].
				" policy.authority=pass policy.authority-uri=".$bimi["record"]["a"].
				" policy.indicator-uri=".$bimi["record"]["l"]." policy.indicator-hash=".sha2($bimi_vmc["indicator"], 256)[:-8].
				" policy.logo-preference=".($bimi["record"]["avp"] == "personal" ? "personal" : "brand"));
		}
		else
		{
			echo "BIMI SVG error: $bimi_vmc; $bimi; $bimi_svg";
			$mail->addHeader("Authentication-Results",
				gethostname()."; bimi=fail header.d=".$bimi["domain"]." header.selector=".$bimi["selector"]);
		}
	}
	else
	{
		echo "BIMI VMC error: $bimi_vmc; $bimi";
		$mail->addHeader("Authentication-Results",
			gethostname()."; bimi=".$bimi["authentication-results"]." header.d=".$bimi["domain"]." header.selector=".$bimi["selector"]);
	}
}
else
{
	if ($bimi["error"] and $bimi["class"] != "dmarc" and $bimi["class"] != "dns")
		echo "BIMI error: $bimi";
	$mail->addHeader("Authentication-Results",
		gethostname()."; bimi=".$bimi["authentication-results"]);
}
/*
// required by some email clients
$mail->signDKIM($selector, $domain, $key,
	["additional_headers" => ["Authentication-Results"], "body_length" => 0]);
*/

/*
// Unverified
if ($bimi["record"]["l"])
{
	$svg = http($bimi["record"]["l"], ["timeout" => 5, "max_file_size" => 512000, "tls_default_ca" => true]);
	if ($svg)
	{
		$bimi_svg = bimi_svg_check($svg); // see plugin
		if ($bimi_svg["valid"])
		{
			$mail->addHeader("BIMI-Indicator",
				str_strip(array_join(pcre_match_all(#/(^(.{0,49})|(.{0,64}))/, base64_encode($svg))[1], "\r\n ")),
				["encode" => false]);
		}
	}
}
*/
```
