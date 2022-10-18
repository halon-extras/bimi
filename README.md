# Brand Indicators for Message Identification

This plugin adds BIMI validation. It supports both VMC (see current ca.cert) and "externally verified" certificates/domains. It can also validates SVG image profiles as required by the BIMI specification. This plugin depends on libxml2 and its validation is based on a RelaxNG scheme.

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
$dmarc = dmarc($mail, $connection["remoteip"], $connection["helo"] ?? "", $transaction["senderaddress"]["domain"] ?? "");
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
			$mail->addHeader("BIMI-Indicator",
				str_strip(array_join(pcre_match_all(#/(^(.{0,49})|(.{0,64}))/, base64_encode($bimi_vmc["indicator"]))[1], "\r\n ")),
				["encode" => false]);
			$mail->addHeader("BIMI-Location",
				"v=BIMI1; l=".$bimi["record"]["l"]." a=".$bimi["record"]["a"]);
			$mail->addHeader("Authentication-Results",
				gethostname()."; bimi=pass header.d=".$bimi["domain"]." header.selector=".$bimi["selector"].
				" policy.authority=pass policy.authority-uri=".$bimi["record"]["a"]);
			/*
			// required by some email clients
			$mail->signDKIM($selector, $domain, $key,
				["additional_headers" => ["Authentication-Results"], "body_length" => 0]);
			*/
		}
		else
		{
			echo "BIMI SVG error: $bimi_vmc; $bimi; $bimi_svg";
		}
	}
	else
		echo "BIMI VMC error: $bimi_vmc; $bimi";
}
else
{
	if ($bimi["error"] and $bimi["class"] != "dmarc" and $bimi["class"] != "dns")
		echo "BIMI error: $bimi";
}

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
