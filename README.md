## Brand Indicators for Message Identification

Usage

```
import $calist from "bimi/ca.crt";
import { bimi, bimi_vmc } from "bimi/bimi.hsl";
import { dmarc } from "dmarc/dmarc.hsl";

$dmarc = dmarc($mail, $senderip, $senderhelo, $senderdomain);
$bimi = bimi($mail, $dmarc);

// Verified Mark Certificate (VMC)
if ($bimi["record"]["a"])
{
	$bimi_vmc = bimi_vmc($bimi, $calist);
	if ($bimi_vmc["indicator"])
	{
		$bimi_svg = bimi_svg_check($bimi_vmc["indicator"]); // see plugin
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
			$mail->signDKIM(...selector, ...domain, ...key,
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
