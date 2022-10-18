# Brand Indicators for Message Identification

This plugin adds BIMI validation. It supports both VMC (see current ca.cert) and "externally verified" certificates/domains. It can also validates SVG image profiles as required by the BIMI specification. This plugin depends on libxml2 and its validation is based on a RelaxNG scheme.

## Installation

1. [Enable root login](https://support.halon.io/hc/en-us/articles/360000333929-Enable-root-login)
2. Download the `halon-extras-bimi-1.0.0-freebsd-12.3-x86_64.zip` file from [here](https://github.com/halon-extras/bimi/releases/tag/1.0.0) and extract it
2. Use SCP to upload the plugin to each Halon MTA node
	* Place the `SVG_PS-latest.rng` file at `/storage/SVG_PS-latest.rng`
	* Place the `bimi.so` file at `/storage/plugins/bimi.so`
	* Place the `lib/libxml2.so` file at `/storage/plugins/lib/libxml2.so`
3. Go in under "Hosts -> Services -> SMTP server" in the web administration and add the plugin on each MTA node with the ID `bimi`
	* Also enable the "HSL plugin" checkbox
4. Add the HSL module to the configuration
	* Add the `main.hsl` file with the ID `bimi/main.hsl`
	* Add the `bimi.hsl` file with the ID `bimi/bimi.hsl`
	* Add the `ca.crt` file with the ID `bimi/ca.crt`
5. Add the [DMARC HSL module](https://github.com/halon-extras/dmarc) to the configuration
	* Add the `main.hsl` file with the ID `dmarc/main.hsl`
	* Add the `dmarc.hsl` file with the ID `dmarc/dmarc.hsl`

## Usage

```
import { dmarc } from "dmarc";
import { bimi, bimi_vmc, bimi_svg_check } from "bimi";

$mail = $arguments["mail"];
$dmarc = dmarc($mail, $connection["remoteip"], $connection["helo"], $transaction["senderaddress"]["domain"]);
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
