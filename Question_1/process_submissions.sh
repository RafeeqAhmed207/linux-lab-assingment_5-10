#!/bin/bash
SUBMIT_DIR="./submissions"
BACKUP_DIR="./backup_$(date +%Y%m%d_%H%M%S)"
REPORT_FILE="report.txt"
ERROR_LOG="errors.log"
HASH_LIST="hashes.tmp"

> "$ERROR_LOG"
> "$HASH_LIST"

if [ ! -d "$SUBMIT_DIR" ]; then
    echo "ERROR: Submission directory '$SUBMIT_DIR' not found." >> "$ERROR_LOG"
    exit 1
fi

mkdir -p "$BACKUP_DIR" 2>>"$ERROR_LOG"

total=0
duplicates=0
backed_up=0

for file in "$SUBMIT_DIR"/*; do
    [ -e "$file" ] || continue
    total=$((total+1))

    hash=$(md5sum "$file" 2>>"$ERROR_LOG" | awk '{print $1}')

    if [ -z "$hash" ]; then
        echo "ERROR: Could not hash file '$file'" >> "$ERROR_LOG"
        continue
    fi

    if grep -q "^$hash\$" "$HASH_LIST" 2>>"$ERROR_LOG"; then
        duplicates=$((duplicates+1))
    else
        echo "$hash" >> "$HASH_LIST"
        cp "$file" "$BACKUP_DIR/" 2>>"$ERROR_LOG"
        if [ $? -eq 0 ]; then
            backed_up=$((backed_up+1))
        else
            echo "ERROR: Failed to back up '$file'" >> "$ERROR_LOG"
        fi
    fi
done

{
    echo "Submission Processing Report"
    echo "Generated: $(date)"
    echo "-----------------------------------"
    echo "Total files processed : $total"
    echo "Duplicate files found  : $duplicates"
    echo "Unique files backed up : $backed_up"
    echo "Backup location        : $BACKUP_DIR"
} > "$REPORT_FILE"

rm -f "$HASH_LIST"

echo "Done. See $REPORT_FILE for summary and $ERROR_LOG for any errors."
