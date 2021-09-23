import sqlite3
import sys

if len(sys.argv) < 3:
    print('Usage:')
    print('  script.py wsmtools_db existing_db')
    exit(0)

get_wsm_cards = '''
    SELECT
        Serial,
        Name_JP,
        Type,
        Color,
        Level,
        Cost,
        Soul,
        Power,
        Triggers
    FROM WeissSchwarzCards
'''
insert_into_wsa = '''
    INSERT INTO 
        cards (
            code,
            name,
            type,
            color,
            level,
            cost,
            soul,
            power,
            triggers
        ) VALUES (?,?,?,?,?,?,?,?,?)
'''

get_wsm_traits = '''
    SELECT
        EN,
        WeissSchwarzCardSerial
    FROM
        WeissSchwarzCards_Traits
'''

find_wsa_trait = '''
    SELECT
        id
    FROM
        traits
    WHERE
        trait = ?
'''

insert_wsa_trait = '''
    INSERT INTO
        traits(trait)
    VALUES
        (?)
'''

find_wsa_card_trait = '''
    SELECT 
        trait_id 
    FROM 
        card_trait
    WHERE 
        card_code = ?
'''

insert_card_trait = '''
    INSERT INTO
        card_trait(card_code, trait_id)
    VALUES
        (?,?)
'''

wsm_con = sqlite3.connect(sys.argv[1])
wsa_con = sqlite3.connect(sys.argv[2])

wsm_cur = wsm_con.cursor()
wsa_cur = wsa_con.cursor()

wsm_cur.execute(get_wsm_cards)

data = wsm_cur.fetchall()

for record in data:
    wsa_cur.execute(insert_into_wsa, (
            record[0],
            record[1],
            record[2],
            record[3],
            record[4],
            record[5],
            record[6],
            record[7],
            record[8]
        )
    )

wsm_cur.execute(get_wsm_traits)

data = wsm_cur.fetchall()

for record in data:
    trait_name = record[0]
    card_serial = record[1]
    wsa_cur.execute(find_wsa_trait, (trait_name,))

    trait_id = wsa_cur.fetchall()

    if not trait_id:
        wsa_cur.execute(insert_wsa_trait, (trait_name,))
        wsa_cur.execute(find_wsa_trait, (trait_name,))
        trait_id = wsa_cur.fetchall()

    trait_id = trait_id[0][0]
    wsa_cur.execute(find_wsa_card_trait, (card_serial,))

    traits_for_card = wsa_cur.fetchall()
    trait_found = False
    for rec in traits_for_card:
        if rec[0] == trait_id:
            trait_found = True
    if not trait_found:
        wsa_cur.execute(insert_card_trait, (card_serial, trait_id,))

wsa_con.commit()


wsm_cur.close()
wsa_cur.close()
