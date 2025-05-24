#!/bin/bash

json='[
  {"language":"AR", "name":"Arabic", "flag":"https://flagcdn.com/w80/sa.png"},
  {"language":"BG", "name":"Bulgarian", "flag":"https://flagcdn.com/w80/bg.png"},
  {"language":"CS", "name":"Czech", "flag":"https://flagcdn.com/w80/cz.png"},
  {"language":"DA", "name":"Danish", "flag":"https://flagcdn.com/w80/dk.png"},
  {"language":"DE", "name":"German", "flag":"https://flagcdn.com/w80/de.png"},
  {"language":"EL", "name":"Greek", "flag":"https://flagcdn.com/w80/gr.png"},
  {"language":"EN-GB", "name":"English (British)", "flag":"https://flagcdn.com/w80/gb.png"},
  {"language":"EN-US", "name":"English (American)", "flag":"https://flagcdn.com/w80/us.png"},
  {"language":"ES", "name":"Spanish", "flag":"https://flagcdn.com/w80/es.png"},
  {"language":"ET", "name":"Estonian", "flag":"https://flagcdn.com/w80/ee.png"},
  {"language":"FI", "name":"Finnish", "flag":"https://flagcdn.com/w80/fi.png"},
  {"language":"FR", "name":"French", "flag":"https://flagcdn.com/w80/fr.png"},
  {"language":"HU", "name":"Hungarian", "flag":"https://flagcdn.com/w80/hu.png"},
  {"language":"ID", "name":"Indonesian", "flag":"https://flagcdn.com/w80/id.png"},
  {"language":"IT", "name":"Italian", "flag":"https://flagcdn.com/w80/it.png"},
  {"language":"JA", "name":"Japanese", "flag":"https://flagcdn.com/w80/jp.png"},
  {"language":"KO", "name":"Korean", "flag":"https://flagcdn.com/w80/kr.png"},
  {"language":"LT", "name":"Lithuanian", "flag":"https://flagcdn.com/w80/lt.png"},
  {"language":"LV", "name":"Latvian", "flag":"https://flagcdn.com/w80/lv.png"},
  {"language":"NB", "name":"Norwegian", "flag":"https://flagcdn.com/w80/no.png"},
  {"language":"NL", "name":"Dutch", "flag":"https://flagcdn.com/w80/nl.png"},
  {"language":"PL", "name":"Polish", "flag":"https://flagcdn.com/w80/pl.png"},
  {"language":"PT-BR", "name":"Portuguese (Brazilian)", "flag":"https://flagcdn.com/w80/br.png"},
  {"language":"PT-PT", "name":"Portuguese (European)", "flag":"https://flagcdn.com/w80/pt.png"},
  {"language":"RO", "name":"Romanian", "flag":"https://flagcdn.com/w80/ro.png"},
  {"language":"RU", "name":"Russian", "flag":"https://flagcdn.com/w80/ru.png"},
  {"language":"SK", "name":"Slovak", "flag":"https://flagcdn.com/w80/sk.png"},
  {"language":"SL", "name":"Slovenian", "flag":"https://flagcdn.com/w80/si.png"},
  {"language":"SV", "name":"Swedish", "flag":"https://flagcdn.com/w80/se.png"},
  {"language":"TR", "name":"Turkish", "flag":"https://flagcdn.com/w80/tr.png"},
  {"language":"UK", "name":"Ukrainian", "flag":"https://flagcdn.com/w80/ua.png"},
  {"language":"ZH", "name":"Chinese (simplified)", "flag":"https://flagcdn.com/w80/cn.png"},
  {"language":"ZH-HA", "name":"Chinese (traditional)", "flag":"https://flagcdn.com/w80/tw.png"},
  {"language":"DEFAULT", "name":"Default", "flag":"https://flagcdn.com/w80/un.png"}
]'

# Convert JSON to Bash array
declare -A flags
while IFS= read -r line; do
  language=$(echo "$line" | jq -r '.language')
  name=$(echo "$line" | jq -r '.name')
  flag=$(echo "$line" | jq -r '.flag')
  flags["$language"]="$name,$flag"
done < <(echo "$json" | jq -c '.[]')

# Create the flags directory if it doesn't exist
mkdir -p ../images/flags

# Download each flag
for language in "${!flags[@]}"; do
  IFS=',' read -r name flag <<< "${flags[$language]}"
  filename="../images/flags/${language}.png"
  if [ ! -f "$filename" ]; then
    echo "Downloading $name flag..."
    curl -s -o "$filename" "$flag"
  else
    echo "$name flag already exists, skipping download."
  fi
done