|----------------------------------------------------------------------------|
| Shaman Functions
|----------------------------------------------------------------------------|
SUB check_Canni
/if (${Debug}) /echo |- autoCanni ==>
	/if (${AutoCanni}) {
	  |only canni if im not in ooc regen
		/if (${Me.CombatState.NotEqual[RESTING]}) {
			/declare s int local
			/for s 1 to ${canniSpells2D.Size[1]}
			  /if (${canniSpells2D[${s},${iCastName}].NotEqual[Spiritual Channeling]} || (${canniSpells2D[${s},${iCastName}].Equal[Spiritual Channeling]} && ${Me.CombatState.Equal[COMBAT]})) {
          /call check_Ready "canniSpells2D" ${s}
          /if (${c_Ready}) {
            /if (${check_Mana["canniSpells2D",${s}]}) {
            	/if (!${canniSpells2D[${s},${iCastName}].Equal[Cannibalization]} || ${Me.CurrentHPs} > 2000) {
								/if (${canniSpells2D[${s},${iMinHP}]} <= ${Me.PctHPs}) {
              		/call e3_Cast ${Me.ID} "canniSpells2D" ${s}
                }
							}
            }
          }
        }
			/if (!${ActionTaken}) /next s
		}
	}
/if (${Debug}) /echo <== autoCanni -|
/RETURN
|----------------------------------------------------------------------------|
Sub SHM_Setup
/if (${Debug}) /echo |- SHM_Setup ==>
	/call iniToVarV "${Character_Ini},Shaman,Auto-Canni (On/Off)" AutoCanni bool outer
	/call IniToArrayV "${Character_Ini},Shaman,Canni#" canniSpells
	/call IniToArrayV "${Character_Ini},Shaman,Idol#" idolSpells
  	/if (${canniSpells.Size}) /call BuildSpellArray "canniSpells"	"canniSpells2D"
  	/if (${idolSpells.Size}) /call BuildSpellArray "idolSpells"	"idolSpells2D"	
/if (${Debug}) /echo <== SHM_Setup -|
/return
|----------------------------------------------------------------------------|
SUB SHM_MacroSettings
/if (${Debug}) /echo |- SHM_MacroSettings ==>
	/call WriteToIni "${advSettings_Ini},SHM Functions,SHM Function#1" "check_Debuffs" 1
	/call WriteToIni "${advSettings_Ini},SHM Functions,SHM Function#2" "check_Burns" 1
	/call WriteToIni "${advSettings_Ini},SHM Functions,SHM Function#3" "check_Heals" 1
	/call WriteToIni "${advSettings_Ini},SHM Functions,SHM Function#4" "check_Cures" 1
  	/call WriteToIni "${advSettings_Ini},SHM Functions,SHM Function#5" "check_Buffs" 1
	/call WriteToIni "${advSettings_Ini},SHM Functions,SHM Function#6" "check_DoTs" 1
	/call WriteToIni "${advSettings_Ini},SHM Functions,SHM Function#7" "check_AE" 1
	/call WriteToIni "${advSettings_Ini},SHM Functions,SHM Function#8" "check_Nukes" 1
	/call WriteToIni "${advSettings_Ini},SHM Functions,SHM Function#9" "check_Pets" 1
	/call WriteToIni "${advSettings_Ini},SHM Functions,SHM Function#10" "check_Canni" 1
/if (${Debug}) /echo <== SHM_MacroSettings -|
/RETURN
|----------------------------------------------------------------------------|
SUB SHM_CharacterSettings
/if (${Debug}) /echo |- SHM_CharacterSettings ==>
	/call WriteToIni "${Character_Ini},Shaman,Auto-Canni (On/Off)"
	/call WriteToIni "${Character_Ini},Shaman,Canni"
	/call WriteToIni "${Character_Ini},Shaman,Idol"
/if (${Debug}) /echo <== SHM_CharacterSettings -|
/RETURN
|----------------------------------------------------------------------------|
Sub SHM_Aliases
  /squelch /alias /Idol /echo Idol
/return
|----------------------------------------------------------------------------|
#event AE_AA "#*# tells you, 'AEAA'"
sub Event_AE_AA(string line)
	/if (${Me.Class.ShortName.Equal[SHM]}) {
   /if (${Me.AltAbilityReady[Mass Group Buff]} && ${Me.AltAbilityReady[Ancestral Aid]}) {
     /bc MGB Ancestral Aid inc...
     /casting "Mass Group Buff|alt" -maxtries|3
     /delay 5 !${Me.AltAbilityReady[Mass Group Buff]}
     /delay 5
     /casting "Ancestral Aid|alt" -maxtries|3
     /delay 5 !${Me.AltAbilityReady[Ancestral Aid]}
     /casting "Ancestral Aid|alt" -maxtries|3
     /rs MGB Ancestral Aid inc...
   } else /if (!${Me.AltAbilityReady[Mass Group Buff]}) {
      /bc Mass Group Buff is not available...
   } else /bc Ancestral Aid is not available...
	}
/return
|----------------------------------------------------------------------------|
| Summons and maintains the malo idol that's configured in the shaman's ini file. Idol= |
#event Idol "[MQ2] Idol"
#event Idol "#1# tells you, 'Idol'"
#event Idol "Soul Idol has expired."
SUB Event_Idol(line, ChatSender)
	/tar Soul Idol
	/delay 5
	/if (${Target.ID} != NULL) { 
		/g Unable to summon idol. One is already active
		/return
	}
	/declare s int local
	/for s 1 to ${idolSpells2D.Size[1]}{
		/call check_Ready "idolSpells2D" ${s}
		/if (${c_Ready}) /call e3_Cast ${Me.ID} "idolSpells2D" ${s}				
	}			
/return
|----------------------------------------------------------------------------|
Sub SHM_Background_Events
	/doevents AE_AA
	/doevents Idol
/return
