|----------------------------------------------------------------------------|
| Druid Functions
|----------------------------------------------------------------------------|
Sub DRU_Setup
	/call iniToVarV "${Character_Ini},Druid,Evac Spell" Evac_Spell string outer
	/call iniToVarV "${Character_Ini},Druid,Auto-Cheetah (On/Off)" autoCheetahOn bool outer
/return
|----------------------------------------------------------------------------|
SUB DRU_MacroSettings
/if (${Debug}) /echo |- DRU_MacroSettings ==>
	/call WriteToIni "${advSettings_Ini},DRU Functions,DRU Function#1" "check_healChain" 1
	/call WriteToIni "${advSettings_Ini},DRU Functions,DRU Function#2" "check_Heals" 1
  /call WriteToIni "${advSettings_Ini},DRU Functions,DRU Function#3" "check_Cures" 1
	/call WriteToIni "${advSettings_Ini},DRU Functions,DRU Function#4" "check_Buffs" 1
	/call WriteToIni "${advSettings_Ini},DRU Functions,DRU Function#5" "check_Burns" 1
	/call WriteToIni "${advSettings_Ini},DRU Functions,DRU Function#6" "check_Debuffs" 1
	/call WriteToIni "${advSettings_Ini},DRU Functions,DRU Function#7" "check_DoTs" 1
	/call WriteToIni "${advSettings_Ini},DRU Functions,DRU Function#8" "check_AE" 1
	/call WriteToIni "${advSettings_Ini},DRU Functions,DRU Function#9" "check_Nukes" 1
	/call WriteToIni "${advSettings_Ini},DRU Functions,DRU Function#10" "check_Food" 1
	/call WriteToIni "${advSettings_Ini},DRU Functions,DRU Function#10" "check_Pets" 1
/RETURN
|----------------------------------------------------------------------------|
SUB DRU_CharacterSettings
	/call WriteToIni "${Character_Ini},Druid,Evac Spell"
/RETURN
|----------------------------------------------------------------------------|
Sub DRU_Aliases
/return
|----------------------------------------------------------------------------|
#event AE_SOW "#*# tells you, 'AESOW'"
sub Event_AE_SOW(string line)
	/if (${Me.Class.ShortName.Equal[DRU]}) {
	 /if (${Me.AltAbilityReady[Mass Group Buff]} && ${Me.AltAbilityReady[Spirit of the Wood]}) {
		 /bc MGB Spirit of the Wood inc...
		 /call castSimpleSpell "Mass Group Buff" 0
		 /call castSimpleSpell "Spirit of the Wood" 0
		 /rs MGB Spirit of the Wood inc...
	 } else /if (!${Me.AltAbilityReady[Mass Group Buff]}) {
			/bc Mass Group Buff is not available...
	 } else /bc Spirit of the Wood is not available...
	}
/return

#event AE_FOE "#*# tells you, 'AEFOE'"
sub Event_AE_FOE(string line)
	/if (${Me.Class.ShortName.Equal[DRU]}) {
	 /if (${Me.AltAbilityReady[Mass Group Buff]} && ${Bool[${Me.Book[Flight of Eagles]}]}) {
		 /bc MGB Flight of Eagles inc...
		 /call castSimpleSpell "Mass Group Buff" 0
		 /call castSimpleSpell "Flight of Eagles" 0
		 /rs MGB Spirit of the Wood inc...
	 } else /if (!${Me.AltAbilityReady[Mass Group Buff]}) {
			/bc Mass Group Buff is not available...
	 } else /bc Flight of Eagles is not available...
	}
/return
|----------------------------------------------------------------------------|
Sub DRU_Background_Events
	/doevents AE_SOW
	/doevents AE_FOE
	/call autoCheetah
/return

| Automatically cast group AA Communion of the Cheetah whenever the buff is missing on the Druid and the druid doesn't have Selo's Sonata
SUB autoCheetah


  /if (${autoCheetahOn} && !${Me.Buff[Selo's Sonata].ID} && !${Me.Buff[Selo's Accelerating Chorus].ID} && (!${Me.Buff[Spirit of Cheetah].ID} || ${Me.Buff[Spirit of Cheetah].Duration.TotalSeconds} < 10) && !${Me.Invis} && ${Me.AltAbilityReady[Communion of the Cheetah]}) {
	/alt act 939
  }
/RETURN