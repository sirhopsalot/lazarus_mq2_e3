|----------------------------------------------------------------------------|
| Necromancer Functions
|----------------------------------------------------------------------------|


SUB check_NecroAggro

  /if (${Me.Feigning}) /return
  /if (!${Assisting}) /return  
  /if (${Bool[${Me.Song[Harm Shield]}]}) /return
  /if (${Bool[${Me.Song[Embalmer's Carapace]}]}) /return
 

  |update our max agro on xtargets
  /declare i int local 1
  /declare aggroCap int local 90
  /declare tempMaxAggro int local 0
  /declare currentAggro int local 0
  /declare mobId int local 0
  |first we do a quick max agro check
  /for i 1 to 13
  /if (!${Me.XTarget[${i}].TargetType.Equal[Auto Hater]}) /continue
    /varset mobId ${Me.XTarget[${i}].ID}
    /if (!${Bool[${mobId}]}) /continue
    /if (${mobId}<1) /continue
    /if (!${Spawn[npc id ${mobId}].Aggressive}) /continue
    |try and avoid xtarget bug by checking to make sure this isn't a corpse before we access the aggro
    /if (${Spawn[npc id ${mobId}].Type.Equal[Corpse]}) /continue
    /varset currentAggro ${Me.XTarget[${i}].PctAggro}
    /if (${tempMaxAggro} <  ${currentAggro}) {
      /varset tempMaxAggro ${currentAggro}
    }
  /next i
  /if (${tempMaxAggro} > ${aggroCap} && !${Bool[${Me.Song[Gathering Dusk]}]}) {
    /bc [+g+] [${Time}]  <check_FD> agro is high, calling Death Peace for ${Me.Name}
    /call check_ReadySimple "Improved Death Peace"
    /if (${c_Ready}) {
        /call castSimpleSpell "Improved Death Peace" 0
        /stand
        /return
    }
    /call check_ReadySimple "Death Peace"
    /if (${c_Ready}) {
        /call castSimpleSpell "Death Peace" 0
        /stand
        /return
    }
  }
  /if (${tempMaxAggro} > ${aggroCap}) {

      /call check_ReadySimple "Harmshield"
      /if (${c_Ready}) {
        /call castSimpleSpell "Harmshield" 0
        /return
       }
      /call check_ReadySimple "Embalmer's Carapace"
      /if (${c_Ready}) {
        /call castSimpleSpell "Embalmer's Carapace" 0
        /return
       }
  }

/return

SUB check_Lich
  /if (${Debug}) /echo |- check_castLich ==>
  |/echo in checklich  ${lichSpell2D[1,${iMinHP}]}, ${Me.PctHPs} ${Me.Buff[${lichSpell2D[1,${iSpellName}]}]}
  /if (${Defined[lichSpell2D]}) {
    /if (${Me.PctHPs} <= 25 || ${lichSpell2D[1,${iMinHP}]} >= ${Me.PctHPs}) {
      /if (${Bool[${Me.Buff[${lichSpell2D[1,${iSpellName}]}]}]}) {
        /echo Removing Lich due to low HP
        /removebuff ${lichSpell2D[1,${iSpellName}]}
      }
    } else {
      /if (!${Bool[${Me.Buff[${lichSpell2D[1,${iSpellName}]}]}]} && !${Bool[${Me.Buff[${lichSpell2D[1,${iCheckFor}]}]}]}) {
        /if (${Spell[${lichSpell2D[1,${iSpellName}]}].Stacks} && ${Spell[${lichSpell2D[1,${iSpellName}]}].NewStacks}) {
          /if (${check_Mana["lichSpell2D",1]}) {
            /call check_Ready "lichSpell2D" 1
            /if (${c_Ready}) {
            |/if (${check_Ready["lichSpell2D",1]}) {
              /if (${lichSpell2D[1,${iMinHP}]} <= ${Me.PctHPs} && ${Me.CurrentHPs} > 100) {
                /call e3_Cast ${Me.ID} "lichSpell2D" 1
              }
            }
          }
        }
      }
    }
  }
  /if (${Debug}) /echo <== check_castLich -|
/return
|----------------------------------------------------------------------------|
SUB check_manaDump
/if (${Debug}) /echo |- check_manaDump ==>
	/if (${manaDump} && !${medBreak} && !${Following}) {
		/declare manaDump_Target string local
		/declare manaDumpSpellIndex int local 0
    /declare i int local
    /declare s int local

    /for s 1 to ${manaDump_Spells2D.Size[1]}
      /call check_Ready "manaDump_Spells2D" ${s}
      /if (${c_Ready}) {
        /if (${check_Mana["manaDump_Spells2D",${s}]}) {
      		/varset manaDumpSpellIndex ${s}
				}
			}
    /if (${manaDumpSpellIndex}==0) /next s

    /if (${manaDumpSpellIndex} > 0 ) {
      /for i 1 to ${whoto_manaDump.Size}
        /if (${check_Distance[${NetBots[${whoto_manaDump[${i}].Arg[1,/]}].ID},${manaDump_Spells2D[${manaDumpSpellIndex},${iMyRange}]}]}) {
        	/if (${NetBots[${whoto_manaDump[${i}].Arg[1,/]}].PctMana} <= ${whoto_manaDump[${i}].Arg[2,/]}) {
          	/call e3_Cast ${NetBots[${whoto_manaDump[${i}].Arg[1,/]}].ID} "manaDump_Spells2D" ${manaDumpSpellIndex}
          }
				}
			/if (!${ActionTaken}) /next i
    }
  }
/if (${Debug}) /echo <== check_manaDump -|
/return

|----------------------------------------------------------------------------|
#event AE_DMF "#*# tells you, 'AEDMF'"
sub Event_AE_DMF(string line)
/if (${Me.Class.ShortName.Equal[NEC]}) {
   /if (${Me.AltAbilityReady[Mass Group Buff]} && ${Bool[${Me.Book[Dead Men Floating]}]}) {
     /bc MGB Dead Men Floating inc...
     /call castSimpleSpell "Mass Group Buff" 0
     /call castSimpleSpell "Dead Men Floating" 0
     /rs MGB Dead Men Floating inc...
   } else /if (!${Me.AltAbilityReady[Mass Group Buff]}) {
      /bc Mass Group Buff is not available...
   } else /bc Dead Men Floating is not in your Spell Book, Go and buy it fool...
}
/return

|----------------------------------------------------------------------------|
Sub NEC_Setup
/if (${Debug}) /echo |- NEC_Setup ==>
	/call IniToArrayV "${Character_Ini},${Me.Class},Lich Spell" lichSpell string outer
	/call IniToArrayV "${Character_Ini},${Me.Class},LifeTap#" lifeTaps
  /call IniToArrayV "${Character_Ini},${Me.Class},Mana Dump#" manaDump_Spells
  /call IniToArrayV "${Character_Ini},${Me.Class},Who to Mana Dump#" whoto_manaDump
  /call iniToVarV "${Character_Ini},${Me.Class},Mana Dump (On/Off)" manaDump bool outer

  /if (${Defined[lichSpell]})       /call BuildSpellArray "lichSpell"	"lichSpell2D"
  /if (${Defined[lifeTaps]})        /call BuildSpellArray "lifeTaps"	"lifeTaps2D"
  /if (${Defined[manaDump_Spells]}) /call BuildSpellArray "manaDump_Spells"	"manaDump_Spells2D"

/if (${Debug}) /echo <== NEC_Setup -|
/return
|----------------------------------------------------------------------------|
SUB NEC_MacroSettings
/if (${Debug}) /echo |- NEC_MacroSettings ==>
	/call WriteToIni "${advSettings_Ini},NEC Functions,NEC Function#1" "check_Burns" 1
	/call WriteToIni "${advSettings_Ini},NEC Functions,NEC Function#2" "check_Lich" 1
	/call WriteToIni "${advSettings_Ini},NEC Functions,NEC Function#3" "check_manaDump" 1
  /call WriteToIni "${advSettings_Ini},NEC Functions,NEC Function#4" "check_Cures" 1
	/call WriteToIni "${advSettings_Ini},NEC Functions,NEC Function#5" "check_Debuffs" 1
	/call WriteToIni "${advSettings_Ini},NEC Functions,NEC Function#6" "check_DoTs" 1
	/call WriteToIni "${advSettings_Ini},NEC Functions,NEC Function#7" "check_lifeTap" 1
	/call WriteToIni "${advSettings_Ini},NEC Functions,NEC Function#8" "check_Nukes" 1
	/call WriteToIni "${advSettings_Ini},NEC Functions,NEC Function#9" "check_Buffs" 1
	/call WriteToIni "${advSettings_Ini},NEC Functions,NEC Function#10" "check_Pets" 1
  /call WriteToIni "${advSettings_Ini},NEC Functions,NEC Function#11" "check_Food" 1
/if (${Debug}) /echo <== NEC_MacroSettings -|
/return
|----------------------------------------------------------------------------|
SUB NEC_CharacterSettings
/if (${Debug}) /echo |- NEC_CharacterSettings ==>
	/call WriteToIni "${Character_Ini},${Me.Class},Lich Spell"
	/call WriteToIni "${Character_Ini},${Me.Class},LifeTap"
	/call WriteToIni "${Character_Ini},${Me.Class},Mana Dump (On/Off)"
  |/call WriteToIni "${Character_Ini},${Me.Class},Mana Dump Engage Pct" 70
	/call WriteToIni "${Character_Ini},${Me.Class},Who to Mana Dump"
	/call WriteToIni "${Character_Ini},${Me.Class},Mana Dump"
/if (${Debug}) /echo <== NEC_CharacterSettings -|
/return
|----------------------------------------------------------------------------|
Sub NEC_Aliases
/if (${Debug}) /echo |- NEC_Aliases ==>
	/noparse /squelch /alias /manadump /bc Mana dump
/if (${Debug}) /echo <== NEC_Aliases -|
/return
|----------------------------------------------------------------------------|
Sub NEC_Background_Events
  /doevents AE_DMF
/return