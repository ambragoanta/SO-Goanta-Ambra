#!/bin/bash

if [ "$#" -ne 1 ]; then
    echo "Utilizare: $0 <$1>"
    exit 1
fi

contor=0

while IFS= read -r linie; do
    if [[ $linie =~ ^[A-Z] && $linie =~ [$1] && $linie =~ [\.!?]$ && ! $linie =~ ,\ și ]]; then
        ((contor++))
    fi
done

echo $contor