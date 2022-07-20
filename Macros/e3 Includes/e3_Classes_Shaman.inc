|----------------------------------------------------------------------------|
| Shaman Functions
|----------------------------------------------------------------------------|

SUB check_Totem


  /if (${Bool[${Me.Song[Inconspicuous Totem]}]}) {

     /notify ShortDurationBuffWindow Buff${Math.Calc[${Me.Song[Totemic Form].ID}-1].Int} leftmouseup
		 /notify ShortDurationBuffWindow Buff${Math.Calc[${Me.Song[Inconspicuous Totem].ID}-1].Int} leftmouseup
    
  }
  
  /if (!${Assisting}) /return  
  |update our max agro on xtargets
  /declare i int local 1
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
    /if (${tempMaxAggro} < ${currentAggro} ) {
      /varset tempMaxAggro  ${currentAggro}
    }
  /next i
  /if (${tempMaxAggro} > 90) {
    /bc [+g+] [${Time}] <check_Totem> agro is high at ${tempMaxAggro}%, calling Inconspicuous Totem for ${Me.Name}
    /call check_ReadySimple "Inconspicuous Totem"
    /if (${c_Ready}) {
        /call castSimpleSpell "Inconspicuous Totem" 0
        /delay 4
        /notify ShortDurationBuffWindow Buff${Math.Calc[${Me.Song[Totemic Form].ID}-1].Int} leftmouseup
		    /notify ShortDurationBuffWindow Buff${Math.Calc[${Me.Song[Inconspicuous Totem].ID}-1].Int} leftmouseup

		}
  }
 

/return

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
            	/if (!${canniSpells2D[${s},${iCastName}].Equal[Cannibalization]} || ${Me.CurrentHPs} > 5600) {
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
  	/if (${canniSpells.Size}) /call BuildSpellArray "canniSpells"	"canniSpells2D"	
/if (${Debug}) /echo <== SHM_Setup -|
/return
|----------------------------------------------------------------------------|
SUB SHM_MacroSettings
/if (${Debug}) /echo |- SHM_MacroSettings ==>
	/call WriteToIni "${advSettings_Ini},SHM Functions,SHM Function#1" "check_Debuffs" 1
	/call WriteToIni "${advSettings_Ini},SHM Functions,SHM Function#2" "check_Heals" 1
	/call WriteToIni "${advSettings_Ini},SHM Functions,SHM Function#3" "check_Cures" 1
  	/call WriteToIni "${advSettings_Ini},SHM Functions,SHM Function#4" "check_Buffs" 1
	/call WriteToIni "${advSettings_Ini},SHM Functions,SHM Function#5" "check_Burns" 1
	/call WriteToIni "${advSettings_Ini},SHM Functions,SHM Function#6" "check_DoTs" 1
	/call WriteToIni "${advSettings_Ini},SHM Functions,SHM Function#7" "check_AE" 1
	/call WriteToIni "${advSettings_Ini},SHM Functions,SHM Function#8" "check_Nukes" 1
	/call WriteToIni "${advSettings_Ini},SHM Functions,SHM Function#9" "check_Pets" 1
	/call WriteToIni "${advSettings_Ini},SHM Functions,SHM Function#10" "check_Canni" 1
	/call WriteToIni "${advSettings_Ini},SHM Functions,SHM Function#11" "check_Food" 1
/if (${Debug}) /echo <== SHM_MacroSettings -|
/RETURN
|----------------------------------------------------------------------------|
SUB SHM_CharacterSettings
/if (${Debug}) /echo |- SHM_CharacterSettings ==>
	/call WriteToIni "${Character_Ini},Shaman,Auto-Canni (On/Off)"
	/call WriteToIni "${Character_Ini},Shaman,Canni"
/if (${Debug}) /echo <== SHM_CharacterSettings -|
/RETURN
|----------------------------------------------------------------------------|
Sub SHM_Aliases
/return
|----------------------------------------------------------------------------|
#event AE_AA "#*# tells you, 'AEAA'"
sub Event_AE_AA(string line)
	/if (${Me.Class.ShortName.Equal[SHM]}) {
   /if (${Me.AltAbilityReady[Mass Group Buff]} && ${Me.AltAbilityReady[Ancestral Aid]}) {
     /bc MGB Ancestral Aid inc...
	 /call castSimpleSpell "Mass Group Buff" 0
     /call castSimpleSpell "Ancestral Aid" 0
     /rs MGB Ancestral Aid inc...
   } else /if (!${Me.AltAbilityReady[Mass Group Buff]}) {
      /bc Mass Group Buff is not available...
   } else /bc Ancestral Aid is not available...
	}
/return
|----------------------------------------------------------------------------|
Sub SHM_Background_Events
	/doevents AE_AA
/return
