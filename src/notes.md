This is a radio app in EFL written in C. Right now it only has hardcoded stations. 

Help me change so that it can search the radio-browser.info API.

Help me change this so that it has a search input that hits: http://de2.api.radio-browser.info/xml/stations/search?name=KCUR


<result>
  <station changeuuid="e4773454-4203-4452-9181-d29cbbd0777e" stationuuid="cc131982-2cd2-4135-95e6-06c6b72bee5f" serveruuid="bad571a7-bbdd-4b66-be36-29407fa57d33" name="KCUR 89.3 | NPR in Kansas City" url="https://18423.live.streamtheworld.com/KCURFMAAC.aac" url_resolved="https://18423.live.streamtheworld.com/KCURFMAAC.aac" homepage="https://www.kcur.org/" favicon="https://www.kcur.org/favicon-32x32.png" tags="kansas city,local news,news,npr" country="The United States Of America" countrycode="US" iso_3166_2="" state="Missouri" language="english" languagecodes="en" votes="12" lastchangetime="2025-08-09 04:51:59" lastchangetime_iso8601="2025-08-09T04:51:59Z" codec="AAC+" bitrate="64" hls="0" lastcheckok="1" lastchecktime="2025-10-10 11:01:09" lastchecktime_iso8601="2025-10-10T11:01:09Z" lastcheckoktime="2025-10-10 11:01:09" lastcheckoktime_iso8601="2025-10-10T11:01:09Z" lastlocalchecktime="2025-10-10 00:55:02" lastlocalchecktime_iso8601="2025-10-10T00:55:02Z" clicktimestamp="" clickcount="0" clicktrend="0" ssl_error="0" geo_lat="39.08287154976034" geo_long="-94.48054969310762" has_extended_info="false"></station></result>
  
The result looks like that. There should be a dropdown that has name, country, language and tag. 

The results should displayt in a list with the information about the station from the API and with the favicon displayed. 

You can use https://docs.enlightenment.org/api/ecore/doc/html/group__Ecore__Con__Url__Group.html

for the HTTP request. 

Add libxml2 as a dep to process the XML response. Prefer using XPath. 

