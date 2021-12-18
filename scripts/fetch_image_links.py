import sqlite3
import sys
import json

if len(sys.argv) < 3:
    print('Usage:')
    print('  script.py wsmtools_db output_file chosen_set')
    print('  set example: KGL')
    exit(0)

get_image_links = '''
    SELECT
        Serial,
        Images
    FROM WeissSchwarzCards
    WHERE Serial like ?
'''

wsm_con = sqlite3.connect(sys.argv[1])

wsm_cur = wsm_con.cursor()

wsm_cur.execute(get_image_links, (sys.argv[3] + '%',))

data = wsm_cur.fetchall()

new_json = []
for record in data:
    json_object = {}
    json_object['code'] = record[0]
    urls = json.loads(record[1])
    urls.append('')
    json_object['url'] = urls[0]
    new_json.append(json_object)

with open(sys.argv[2], "w") as file:
    file.write(json.dumps(new_json))