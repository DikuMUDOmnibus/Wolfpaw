CC      = gcc
PROF    = 
#Uncomment the next line if you want request support
C_FLAGS = -O -g3 -Wall -Wuninitialized $(PROF)
#Uncomment if your using glibc6+
L_FLAGS = $(PROF)
#Uncomment the next line for libc5
#L_FLAGS = $(PROF)

O_FILES = act_comm.o act_info.o act_move.o act_obj.o act_wiz.o boards.o \
	  build.o clans.o comm.o comments.o const.o db.o deity.o fight.o \
	  ibuild.o handler.o hashstr.o id.o interp.o magic.o makeobjs.o \
	  misc.o mpxset.o mud_comm.o mud_prog.o player.o requests.o \
	  reset.o save.o shops.o skills.o special.o tables.o track.o update.o \
	  grub.o extras.o remort.o arena.o alias.o bank.o \
          quest.o zones.o race.o mail.o wp-olc.o new_mp.o \
	  refferal.o pipe.o act_wiz2.o laws.o web-server.o quest-master.o \
	  death.o smart-mobs.o new-clans.o cgodb.o petitions.o \
	  vehicle.o projectile-attacks.o plog.o save-rooms.o db2.o aliens.o \
	  glory-store.o limits.o magic2.o act_obj2.o color-mapping.o \
	  personal-socials.o player-kill.o ext_bv.o \
          hoarder.o topten.o hosts.o casino.o maim.o upstat.o

C_FILES = act_comm.c act_info.c act_move.c act_obj.c act_wiz.c boards.c \
	  build.c clans.c comm.c comments.c const.c db.c deity.c fight.c \
	  ibuild.c handler.c hashstr.c id.c interp.c magic.c makeobjs.c \
	  misc.c mpxset.c mud_comm.c mud_prog.c player.c requests.c \
       reset.c save.c shops.c skills.c special.c tables.c track.c update.c \
	  grub.c extras.c remort.c arena.c alias.c bank.c \
          quest.c zones.c race.c mail.c wp-olc.c new_mp.c \
          refferal.c pipe.c act_wiz2.c laws.c web-server.c quest-master.c \
	  death.c cps.c smart-mobs.c new-clans.c cgodb.c keeper.c petitions.c \
	  vehicle.c projectile-attacks.c plog.c save-rooms.c db2.c aliens.c \
	  glory-store.c limits.c magic2.c act_obj2.c color-mapping.c \
	  personal-socials.c pshops.c pcastles.c player-kill.c ext_bv.c \
          hoarder.c topten.c hosts.c casino.c maim.c upstat.c

H_FILES = mud.h bet.h keeper.h

all:
#	co $(H_FILES)
	make Wolfpaw4
#	rm -f $(H_FILES)

Wolfpaw4: $(O_FILES)
	@echo "[**] Making Wolfpaw 4.0 Release [**]"
	rm -f Wolfpaw4
	./util/compile_who
	./util/compile_version
	./util/compile_time
	$(CC) $(L_FLAGS) -o Wolfpaw4 $(O_FILES)
	@echo "[**] Completed, type: make install to make it live (then warmboot)"

.c.o: mud.h
	$(CC) -c $(C_FLAGS) $<

clean:
	rm -f $(O_FILES)
	@echo "[**] Source prepared for Clean make. [**]"

install:
	mv -f ../bin/Wolfpaw4 ../bin/Wolfpaw4.old
	cp Wolfpaw4 ../bin
	@echo "[**] New Wolfpaw4 Binary Installed, old one is Wolfpaw4.old [**]"
