import os
os.environ.setdefault("DJANGO_SETTINGS_MODULE", "orm.settings")

import django
django.setup()

def load_color():
    from accbball.models import COLOR
    file = open('color.txt', 'r')
    Lines = file.readlines()
    for line in Lines:
        elements = line.split(' ')
        COLOR.objects.create(NAME = elements[1][:-1])
    file.close()
    return

def load_state():
    from accbball.models import STATE
    file = open('state.txt', 'r')
    Lines = file.readlines()
    for line in Lines:
        elements = line.split(' ')
        STATE.objects.create(NAME = elements[1][:-1])
    file.close()
    return

def load_team():
    from accbball.models import COLOR,STATE,TEAM
    file = open('team.txt', 'r')
    Lines = file.readlines()
    for line in Lines:
        elements = line.split(' ')
        TEAM.objects.create(NAME = elements[1], STATE_ID = STATE.objects.get(STATE_ID = elements[2]), COLOR_ID = COLOR.objects.get(COLOR_ID = elements[3]), WINS = elements[4], LOSSES = elements[5])
    file.close()
    return

def load_player():
    from accbball.models import TEAM, PLAYER
    file = open('player.txt', 'r')
    Lines = file.readlines()
    for line in Lines:
        elements = line.split(' ')
        PLAYER.objects.create(TEAM_ID = TEAM.objects.get(TEAM_ID = elements[1]), UNIFORM_NUM = elements[2], FIRST_NAME = elements[3], LAST_NAME = elements[4], MPG = elements[5], PPG = elements[6], RPG = elements[7], APG = elements[8], SPG = elements[9], BPG = elements[10])
    file.close()
    return


def main():
    load_color()
    load_state()
    load_team()
    load_player()
if __name__ == "__main__":
    main()
