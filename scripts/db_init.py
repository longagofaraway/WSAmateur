import sqlite3
import sys

if len(sys.argv) < 2:
    print('Usage:')
    print('  db_init.py path_to_new_db')
    exit(0)

con = sqlite3.connect(sys.argv[1])
cur = con.cursor()


# cards TABLE
# code - i.e. 5HY/W83-001
# name - only jp names
# type - 0 - character, 1 - event, 2 - climax
# color - 0 - Yellow, 1 - Green, 2 - Red, 3 - Blue, 4 - Purple
# triggers - comma-separated list of triggers 
#   (Soul, Shot, Bounce, Choice, GoldBar, Bag, Door, Standby, Book, Gate)
#   double soul is written as Soul,Soul
# abilities - array of binary data in the form of:
#   1 byte length of array | 2 bytes size of the next ability LE | n bytes ability | 2 bytes size ...
# counter - 1 - card can be played during counter phase (backup/counter), else 0 or NULL. Filled by hand
# card_references - comma-separated list of card codes, whose names are mentioned in the effects text.
#   This is used to show the pictures of those cards, because their names are in jp. Filled by hand

cur.execute('''
CREATE TABLE cards (
    code TEXT PRIMARY KEY,
    name TEXT NOT NULL,
    type INTEGER NOT NULL,
    color INTEGER NOT NULL,
    level INTEGER,
    cost INTEGER,
    soul INTEGER,
    power INTEGER,
    triggers TEXT,
    abilities BLOB,
    counter INTEGER,
    card_references TEXT)''')

# card_trait and traits TABLE

cur.execute('''
CREATE TABLE card_trait (
    card_code TEXT NOT NULL,
    trait_id INTEGER
)''')

cur.execute('''
CREATE TABLE traits (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    trait TEXT NOT NULL
)''')

con.close()
