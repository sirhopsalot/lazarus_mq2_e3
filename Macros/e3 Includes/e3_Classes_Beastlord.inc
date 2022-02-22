|----------------------------------------------------------------------------|
| Beastlord Functions
|----------------------------------------------------------------------------|
Sub BST_Aliases
/return
|----------------------------------------------------------------------------|
Sub BST_MacroSettings
/if (${Debug}) /echo |- BST_MacroSettings ==>
	/call WriteToIni "${advSettings_Ini},BST Functions,BST Function#1" "check_Burns" 1
	/call WriteToIni "${advSettings_Ini},BST Functions,BST Function#2" "check_Debuffs" 1
	/call WriteToIni "${advSettings_Ini},BST Functions,BST Function#3" "check_Heals" 1
	/call WriteToIni "${advSettings_Ini},BST Functions,BST Function#4" "check_Cures" 1
	/call WriteToIni "${advSettings_Ini},BST Functions,BST Function#5" "check_Buffs" 1
	/call WriteToIni "${advSettings_Ini},BST Functions,BST Function#6" "check_Nukes" 1
	/call WriteToIni "${advSettings_Ini},BST Functions,BST Function#7" "check_Pets" 1
	/call WriteToIni "${advSettings_Ini},BST Functions,BST Function#8" "check_DoTs" 1
	/call WriteToIni "${advSettings_Ini},BST Functions,BST Function#9" "check_Food" 1
/if (${Debug}) /echo <== BST_MacroSettings -|
/return
|----------------------------------------------------------------------------|
Sub BST_CharacterSettings
	/call WriteToIni "${Character_Ini},Auto Paragon,Auto Paragon (On/Off)" Off
	/call WriteToIni "${Character_Ini},Auto Paragon,Paragon Spell"
	/call WriteToIni "${Character_Ini},Auto Paragon,Paragon Mana (Pct)"
	/call WriteToIni "${Character_Ini},Auto Paragon,Auto Focused Paragon (On/Off)" Off
	/call WriteToIni "${Character_Ini},Auto Paragon,Focused Paragon Spell"
	/call WriteToIni "${Character_Ini},Auto Paragon,Character"
/return
|----------------------------------------------------------------------------|
Sub BST_Setup
	/declare castParagon bool outer FALSE
	/declare focusedParagonRange int outer 200
	/call iniToVarV "${Character_Ini},Auto Paragon,Auto Paragon (On/Off)" autoParagon bool outer
	/call IniToArrayV "${Character_Ini},Auto Paragon,Paragon Spell" paragonSpell
	/call iniToVarV "${Character_Ini},Auto Paragon,Paragon Mana (Pct)" paragonManaPct int outer
	/call iniToVarV "${Character_Ini},Auto Paragon,Auto Focused Paragon (On/Off)" autoFocusedParagon bool outer
	/call IniToArrayV "${Character_Ini},Auto Paragon,Focused Paragon Spell" focusedParagonSpell
	/call IniToArrayV "${Character_Ini},Auto Paragon,Character#" characterArray
	/if (${Defined[paragonSpell]}) /call BuildSpellArray "paragonSpell" "paragonSpell2D"
	/if (${Defined[focusedParagonSpell]}) /call BuildSpellArray "focusedParagonSpell" "focusedParagonSpell2D"
/return
|----------------------------------------------------------------------------|
Sub BST_Background_Events
	/doevents AE_POS
	/doevents paragonActive
	/doevents paragonToggle
	/doevents focusedParagonToggle
	/call checkParagon
	/call checkFocusedParagon
/return
|----------------------------------------------------------------------------|
#event AE_POS "#*# tells you, 'AEPOS'"
Sub event_AE_POS(string line)
	/if (${Me.Class.ShortName.Equal[BST]}) {
		/if (${Me.AltAbilityReady[Mass Group Buff]} && ${Me.AltAbilityReady[Paragon of Spirit]}) {
			/bc MGB Paragon of Spirit inc...
			/call castSimpleSpell "Mass Group Buff" 0
			/call castSimpleSpell "Paragon of Spirit" 0
			/rs MGB Paragon of Spirit inc...
		} else /if (!${Me.AltAbilityReady[Mass Group Buff]}) {
			/bc Mass Group Buff is not available...
		} else /bc Paragon of Spirit is not available...
	}
/return
|--------------------------------------------------------------|
#event paragonActive "#1# tells you, 'paragon active'"
#event paragonActive "<#1#> paragon active"
#event paragonActive "#1# tells the group, 'paragon active'"
Sub event_paragonActive(line, ChatSender)	
	/if (${autoParagon}) {
		/docommand ${ChatToggle} ${Me}: Auto Paragon is currently on
	} else {
		/docommand ${ChatToggle} ${Me}: Auto Paragon is currently off
	}
	
	/if (${autoFocusedParagon}) {
		/docommand ${ChatToggle} ${Me}: Auto Focused Paragon is currently on
	} else {
		/docommand ${ChatToggle} ${Me}: Auto Focused Paragon is currently off
	}
/return
|----------------------------------------------------------------------------|
|--------------------------------------------------------------|
#event paragonToggle "#1# tells you, 'paragon toggle'"
#event paragonToggle "<#1#> paragon toggle"
#event paragonToggle "#1# tells the group, 'paragon toggle'"
Sub event_paragonToggle(line, ChatSender)
	/if (${autoParagon}) {
		/varset autoParagon FALSE
	} else {
		/varset autoParagon TRUE
	}
	
	/if (${autoParagon}) {
		/docommand ${ChatToggle} ${Me}: Paragon On
	} else {
		/docommand ${ChatToggle} ${Me}: Paragon Off
	}
/return
|--------------------------------------------------------------|
#event focusedParagonToggle "#1# tells you, 'focused paragon toggle'"
#event focusedParagonToggle "<#1#> focused paragon toggle"
#event focusedParagonToggle "#1# tells the group, 'focused paragon toggle'"
Sub event_focusedParagonToggle(line, ChatSender)
	/if (${autoFocusedParagon}) {
		/varset autoFocusedParagon FALSE
	} else {
		/varset autoFocusedParagon TRUE
	}
	
	/if (${autoFocusedParagon}) {
		/docommand ${ChatToggle} ${Me}: Focused Paragon On
	} else {
		/docommand ${ChatToggle} ${Me}: Focused Paragon Off
	}
/return
|----------------------------------------------------------------------------|
Sub checkParagon
	/if (${Group} && ${Me.AltAbilityReady[${paragonSpell[1]}]} && ${autoParagon}) {
		/call checkParagonGroupCasting "paragonSpell2D" 1
		/if (${castParagon} && ${Me.AltAbilityReady[${paragonSpell[1]}]}) {
			/call e3_Cast ${Me.ID} "paragonSpell2D" 1
		}
	}
/return
|----------------------------------------------------------------------------|
Sub checkFocusedParagon
	/declare characterName string local NULL
	/declare manaPct int local NULL
	
	/if (${Me.AltAbilityReady[${focusedParagonSpell[1]}]} && ${autoFocusedParagon}) {
		/declare i int local
		/for i 1 to ${characterArray.Size} {
			/varset characterName ${characterArray[${i}].Arg[1,|]}
			/varset manaPct ${characterArray[${i}].Arg[2,|]}
			
			/if (${Me.AltAbilityReady[${focusedParagonSpell[1]}]} && ${check_Distance[${NetBots[${characterName}].ID},${Spell[${focusedParagonSpell[1]}].MyRange}]} && ${NetBots[${characterName}].PctMana} < ${manaPct} && !${Bool[${NetBots[${characterName}].ShortBuff.Find[${Spell[${focusedParagonSpell[1]}].ID}]}]}) {
				/call e3_Cast ${NetBots[${characterName}].ID} "focusedParagonSpell2D" 1
			}
		}
		/next i
	}
/return
|----------------------------------------------------------------------------|
Sub checkParagonGroupCasting(ArrayName, int ArrayIndex)
	/varset castParagon FALSE
	/declare avgGrpMana	int local ${Me.PctMana}
	/declare numLowMana		int local 0
	/declare targetCount	int local 1
	/declare g				int local
	
	/if (${Me.PctMana} <= ${${ArrayName}[${ArrayIndex},${paragonManaPct}]}) /varset numLowMana 1
	
	| Check range of group members, count number below manapct and add their mana together
	/for g 1 to ${Group}
		/if (${check_Distance[${Group.Member[${g}].ID},${${ArrayName}[${ArrayIndex},${iMyRange}]}]}) {
			/varcalc targetCount ${targetCount} + 1
			/varcalc avgGrpMana ${avgGrpMana} + ${NetBots[${Group.Member[${g}]}].PctMana}
			/if (${Bool[${NetBots[${Group.Member[${g}]}].ID}]} && ${NetBots[${Group.Member[${g}]}].PctMana}  <= ${paragonManaPct}) {
				/varcalc numLowMana ${numLowMana} + 1
			}
		}
	/next g
	
	| Must have at least 3 targets and avg group mana below manapct
	/varcalc avgGrpMana ${avgGrpMana} / ${targetCount}
	/if (${numLowMana} > 2 && ${avgGrpMana} <= ${paragonManaPct}) { 
		/varset castParagon TRUE
	}
/return