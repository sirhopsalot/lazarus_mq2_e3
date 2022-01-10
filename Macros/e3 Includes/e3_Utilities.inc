|----------------------------------------------------
Sub IniToVarV(Ini_Key, VarToMake, VarType)
  /if (${Defined[${VarToMake}]}) /deletevar ${VarToMake}
  /if (${VarType.Equal[bool]}) {
    /declare ${VarToMake} bool outer ${If[${Select[${IniExt[${Ini_Key}]},TRUE,ON,YES,1]},TRUE,FALSE]}
    /return ${${VarToMake}}
  } else {
    /if (${IniExt[${Ini_Key}].Length}) {
      /declare ${VarToMake} ${VarType} outer ${IniExt[${Ini_Key}]}
      /return ${${VarToMake}}
    }
  }
/return

|-----------------------
Sub IniToArrayV(IniKey, ArrayName)
  /declare i int local 1
  /declare j int local 1
  /declare array_size int local 0
  /declare ArrayScope string local outer
  /if (${IniKey.Right[1].Equal[#]} && ${IniExt[${IniKey}1].Length}) {
    /call IniToArray_Deprecated "${IniKey}" "${ArrayName}"
    /return
  }
  /if (${IniKey.Right[1].Equal[#]}) /varset IniKey ${IniKey.Left[-1]}
  |/echo IniKey = ${IniKey} count = ${IniExt[${IniKey}].Count}
  /if (${Defined[${ArrayName}]}) /deletevar ${ArrayName}
  /if (${IniExt[${IniKey}].Index[1].Length}) {
    /for i 1 to ${IniExt[${IniKey}].Count}
    /if (${IniExt[${IniKey}].Index[${i}].Length}) {
      /varcalc array_size ${array_size}+1
    }
    /next i
    |/declare ${ArrayName}[${IniExt[${IniKey}].Count}] string ${ArrayScope}
    /declare ${ArrayName}[${array_size}] string outer
    /if (${IniExt[${IniKey}].Count}) {
      /for i 1 to ${IniExt[${IniKey}].Count}
      |/echo ${ArrayName}[${i}] ${IniExt[${IniKey}].Index[${i}]} Length: ${IniExt[${IniKey}].Index[${i}].Length}
      /if (${IniExt[${IniKey}].Index[${i}].Length}) {
        /varset ${ArrayName}[${j}] ${IniExt[${IniKey}].Index[${i}]}
        /varcalc j ${j}+1
      }
      /next i
    }
  }
/return

|-----------------------
Sub IniToArray_Deprecated(IniKey, ArrayName)
  /declare i int local 1
|/declare IniKey ${File},${Section},${Key}
  /declare ArrayScope string local outer
  /if (${Defined[${ArrayName}]}) /deletevar ${ArrayName}
  /if (${Ini[${IniKey}1].Length}) {
    /declare count int local 0
    :NextKey
    /if (${Ini[${IniKey}${i}].Length}) {
      /varset count ${i}
      /varcalc i ${i}+1
      /goto :NextKey
    }
    /declare ${ArrayName}[${count}] string ${ArrayScope}
    /for i 1 to ${count}
    /varset ${ArrayName}[${i}] ${Ini[${IniKey}${i}]}
    /next i
  |} else {
  |  /declare ${ArrayName}[0] string ${ArrayScope}
  }
/return

SUB IniToVar_Deprecated(Ini_Key, VarToMake, VarType, VarScope)
	/if (${Debug}) /echo |- IniToVar [${Ini_Key}] ==>
| Check for the specified ini key.
	/if (!${Ini[${Ini_Key}].Length}) {
		/if (${Debug}) /echo [${Ini_Key}] not found.
	} else {
		| Validate bool.
		/if (${VarType.Equal[bool]}) {
			/if (${Defined[${VarToMake}]}) /deletevar ${VarToMake}
			/if (!${Defined[new_VarName]}) /declare new_VarName string outer
			/declare ${VarToMake} bool ${VarScope} ${If[${Select[${Ini[${Ini_Key}]},TRUE,ON,YES,1]},TRUE,FALSE]}
			/if (${Debug}) /echo |- IniToVar -| Defined: ${VarToMake} [${${VarToMake}}]
			| Validate int.
		} else /if (${VarType.Equal[Int]}) {
			/if (${Defined[${VarToMake}]}) /deletevar ${VarToMake}
			/declare ${VarToMake} int ${VarScope} ${Ini[${Ini_Key}]}
			/if (${Debug}) /echo |- IniToVar -| Defined: ${VarToMake} [${${VarToMake}}]
			| Validate float.
		} else /if (${VarType.Equal[Float]}) {
			/if (${Defined[${VarToMake}]}) /deletevar ${VarToMake}
			/declare ${VarToMake} float ${VarScope} ${Ini[${Ini_Key}]}
			/if (${Debug}) /echo |- IniToVar -| Defined: ${VarToMake} [${${VarToMake}}]
			| Create string or timer.
		} else {
			/if (${Defined[${VarToMake}]}) /deletevar ${VarToMake}
			/declare ${VarToMake} ${VarType} ${VarScope} ${Ini[${Ini_Key}]}
			/if (${Debug}) /echo |- IniToVar -| Defined: ${VarToMake} [${${VarToMake}}]
		}
	}

	/if (${Debug}) /echo <== IniToVar -|
/RETURN
|-----------------------
|- Creates ini entries from supplied ini keys.
|- Allows the user to toggle whether or not to overwrite an existing entry
|-----------------------
|- @param Ini_Key: The ini file, section, and entry to write to
|- @param WhatToWrite: Data to write to the given Ini_Key
|- @param OverWrite(bool): Allow overwrite
|-----------------------
|- Syntax: /call WriteToIni "Ini_Key" "WhatToWrite" "OverWrite"
|-	Ex:
|-		Blank entry 				- 	/call WriteToIni "Loot.ini,B,Bone Chips"
|-		Entry with setting 			- 	/call WriteToIni "Loot.ini,B,Bone Chips" "Keep"
|-		Overwrite existing entry	- 	/call WriteToIni "Loot.ini,B,Bone Chips" "Destroy" TRUE
|-----------------------
SUB WriteToIni(Ini_Key, WhatToWrite, bool OverWrite)
/if (${Debug}) {
	/echo |- WriteToIni ==>
	/echo Ini_Key = [${Ini_Key}] - Data to write = [${WhatToWrite}] - OverWrite = [${Bool[${OverWrite}]}]	
}

	| By switching ':'s to '*'s in arguemnt 3, to avoid issues when reading variables from the inis.
	/if (${Ini_Key.Arg[3,,].Find[:]}) /varset Ini_Key ${Ini_Key.Arg[1,,]},${Ini_Key.Arg[2,,]},${Ini_Key.Arg[3,,].Replace[:,;]}
	
	| If the Ini_Key already exists, check overwrite	
	/if (${Ini[${Ini_Key}].Length}) {
		/if (${OverWrite}) /goto :OverWrite
	} else {
		:OverWrite
		/if (${Defined[WhatToWrite]}) {
			/ini "${Ini_Key.Arg[1,,]}" "${Ini_Key.Arg[2,,]}" "${Ini_Key.Arg[3,,]}" "${WhatToWrite}"
		
		| Else, create the entry only.
		} else {
			/ini "${Ini_Key.Arg[1,,]}" "${Ini_Key.Arg[2,,]}" "${Ini_Key.Arg[3,,]}"
		}
	}
/if (${Debug}) /echo <== WriteToIni -|
/RETURN

|--------------------------------------------------------|
|- Creates new arrays, or stacks onto existing arrays.	-|
|- Syntax: /call BuildArray ArrayName Data				      -|
|-  Ex: /call BuildArray DebuffTargets ${Target.ID}		  -|
|--------------------------------------------------------|
SUB BuildArray(ArrayName, Data)
/if (${Debug}) /echo |- BuildArray Adding [${ArrayName}-${Data}] ==>
	| If the array is not defined, declare it.
	/if (!${Defined[${ArrayName}]}) {
		/declare ${ArrayName}[1] string outer
		/varset ${ArrayName}[1] ${Data}
	| Else, if the array is already defined.
	} else {
		| Create a copy of the existing array.
		/declare ${ArrayName}Copy[${${ArrayName}.Size}] string local
		/declare i int local
		/for i 1 to ${${ArrayName}.Size}
			/varset ${ArrayName}Copy[${i}] ${${ArrayName}[${i}]}
		/next i
		| Delete the existing array.
		/deletevar ${ArrayName}
		| Build a resized array.
		/declare ${ArrayName}[${Math.Calc[${${ArrayName}Copy.Size}+1].Int}] string outer
		/for i 1 to ${${ArrayName}.Size}
			/varset ${ArrayName}[${i}] ${${ArrayName}Copy[${i}]}
		/next i
		| Insert new data into the array
		/varset ${ArrayName}[${${ArrayName}.Size}] ${Data}
	}
/if (${Debug}) /echo <== BuildArray -|
/RETURN ${ArrayName}

|--------------------------------------------------------|
|- Creates a new array from a delimited string			-|
|--------------------------------------------------------|
SUB ListToArray(ArrayName, Data, delimiter)
	/declare i int local
	/declare arraySize int local
	/varcalc arraySize ${Data.Count[${delimiter}]} +1
	/if (!${Defined[${ArrayName}]}) /declare ${ArrayName}[${arraySize}] string outer
	/for i 1 to ${arraySize}
		/varset ${ArrayName}[${i}] ${Data.Arg[${i},${delimiter}]}
	/next i
/RETURN ${ArrayName}


|---------------------------------------------------------------------|
|- Define variables used by BuildSpellArray and called in e3_Setup
|---------------------------------------------------------------------|
Sub BuildSpellArrayDefinedValues()

	/if (!${Defined[SpellProp]}) {
		/declare SpellProp[43] string outer 0
		/varset SpellProp[1] CastName
		/varset SpellProp[2] CastType
		/varset SpellProp[3] TargetType
		/varset SpellProp[4] SpellGem
		/varset SpellProp[5] SubToRun
		/varset SpellProp[6] GiveUpTimer
		/varset SpellProp[7] MaxTries
		/varset SpellProp[8] CheckFor
		/varset SpellProp[9] Duration
		/varset SpellProp[10] RecastTime
		/varset SpellProp[11] RecoveryTime
		/varset SpellProp[12] MyCastTime
		/varset SpellProp[13] MyRange
		/varset SpellProp[14] Mana
		/varset SpellProp[15] MinMana
		/varset SpellProp[16] MaxMana
		/varset SpellProp[17] MinHP
		/varset SpellProp[18] HealPct
		/varset SpellProp[19] Reagent
		/varset SpellProp[20] ItemMustEquip
		/varset SpellProp[21] SpellName
		/varset SpellProp[22] NoBurn
		/varset SpellProp[23] NoAggro
		/varset SpellProp[24] Mode
		/varset SpellProp[25] Rotate
		/varset SpellProp[26] Delay
		/varset SpellProp[27] CastID
		/varset SpellProp[28] MinEnd
		/varset SpellProp[29] CastInvis
		/varset SpellProp[30] SpellType
		/varset SpellProp[31] CastTarget
    	/varset SpellProp[32] GiftOfMana
    	/varset SpellProp[33] CheckForID
    	/varset SpellProp[34] SpellID
		/varset SpellProp[35] PctAggro
		/varset SpellProp[36] Zone
    	/varset SpellProp[37] MinSick
		/varset SpellProp[38] AllowSpellSwap
		/varset SpellProp[39] NoEarlyRecast
		/varset SpellProp[40] NoStack
		/varset SpellProp[41] TriggerSpell
		|----------------------------
    	/varset SpellProp[42] Ifs
	}
	/if (!${Defined[iCastName]})		/declare iCastName int outer 1
	/if (!${Defined[iCastType]})		/declare iCastType int outer 2
	/if (!${Defined[iTargetType]})		/declare iTargetType int outer 3
	/if (!${Defined[iSpellGem]})		/declare iSpellGem int outer 4
	/if (!${Defined[iSubToRun]})		/declare iSubToRun int outer 5
	/if (!${Defined[iGiveUpTimer]})		/declare iGiveUpTimer int outer 6
	/if (!${Defined[iMaxTries]})		/declare iMaxTries int outer 7
	/if (!${Defined[iCheckFor]})		/declare iCheckFor int outer 8
	/if (!${Defined[iDuration]})		/declare iDuration int outer 9
	/if (!${Defined[iRecastTime]})		/declare iRecastTime int outer 10
	/if (!${Defined[iRecoveryTime]})	/declare iRecoveryTime int outer 11
	/if (!${Defined[iMyCastTime]})		/declare iMyCastTime int outer 12
	/if (!${Defined[iMyRange]})			/declare iMyRange int outer 13
	/if (!${Defined[iMana]})			/declare iMana int outer 14
	/if (!${Defined[iMinMana]})			/declare iMinMana int outer 15
	/if (!${Defined[iMaxMana]})			/declare iMaxMana int outer 16
	/if (!${Defined[iMinHP]})			/declare iMinHP int outer 17
	/if (!${Defined[iHealPct]})			/declare iHealPct int outer 18
	/if (!${Defined[iReagent]})			/declare iReagent int outer 19
	/if (!${Defined[iItemMustEquip]})	/declare iItemMustEquip int outer 20
	/if (!${Defined[iSpellName]})		/declare iSpellName int outer 21
	/if (!${Defined[iNoBurn]})			/declare iNoBurn int outer 22
	/if (!${Defined[iNoAggro]})			/declare iNoAggro int outer 23
	/if (!${Defined[iMode]})			/declare iMode int outer 24
	/if (!${Defined[iRotate]})		/declare iRotate int outer 25
	/if (!${Defined[iDelay]})			/declare iDelay int outer 26	
	/if (!${Defined[iCastID]})			/declare iCastID int outer 27
	/if (!${Defined[iMinEnd]})			/declare iMinEnd int outer 28
	/if (!${Defined[iCastInvis]})		/declare iCastInvis int outer 29
	/if (!${Defined[iSpellType]})		/declare iSpellType int outer 30
	/if (!${Defined[iCastTarget]})		/declare iCastTarget int outer 31
  	/if (!${Defined[iGiftOfMana]})		/declare iGiftOfMana int outer 32
  	/if (!${Defined[iCheckForID]})		/declare iCheckForID int outer 33
  	/if (!${Defined[iSpellID]})		/declare iSpellID int outer 34
  	/if (!${Defined[iPctAggro]})		/declare iPctAggro int outer 35
	/if (!${Defined[iZone]})		/declare iZone int outer 36
  	/if (!${Defined[iMinSick]})		/declare iMinSick int outer 37
	/if (!${Defined[iAllowSpellSwap]})		/declare iAllowSpellSwap int outer 38
	/if (!${Defined[iNoEarlyRecast]})		/declare iNoEarlyRecast int outer 39
	/if (!${Defined[iNoStack]})		/declare iNoStack int outer 40
	/if (!${Defined[iTriggerSpell]})		/declare iTriggerSpell int outer 41
  	/if (!${Defined[iIfs]})		/declare iIfs int outer 42

/return

|--------------------------------------------------------|
|- 2D array for spell casting/full e3_casting use
|--------------------------------------------------------|

|Note, for whatever reason when chaining methods in mq2, you cannot have the same variable name to your sub call as from your input. ??????
Sub BuildSpellArray(ArrayNameFromBuildSpellArray, NewArrayNameFromBuildSpellArray)

	/call BuildSpellArrayBase "${ArrayNameFromBuildSpellArray}" "${NewArrayNameFromBuildSpellArray}" FALSE

/return

SUB BuildSpellArrayBase(ArrayName, NewArrayName, SkipParams)
	
	/if (${Debug}) /echo Array ${ArrayName} ${NewArrayName}
	/if (${Defined[${NewArrayName}]}) /deletevar ${NewArrayName}
	
	/declare ${NewArrayName}[${${ArrayName}.Size},${SpellProp.Size}] string outer 0
	|/echo NA ${NewArrayName} ${ArrayName} ${${ArrayName}.Size} ${SpellProp.Size} ${${NewArrayName}.Size[1]}
	/declare errMsg string local Review entry and restart macro

	/declare i int local
	|for item information lookup optimization
	/declare invSlot int local 0
	/declare bagSlot int local 0

	/declare printAll bool local False
	/declare printMin bool local False
	|first loop through array to ensure i can identify all listed spells
	/declare tmp_castname string local
	|check to see if the spell/aa/etc actually exists.
	/for i 1 to ${${ArrayName}.Size}
		
		/varset tmp_castname ${${ArrayName}[${i}].Arg[1,/]}
		|First check for super common items to ignore this
		/if (!(${tmp_castname.Equal[Molten Orb]} || ${tmp_castname.Equal[Orb of Shadows]}|| ${tmp_castname.Equal[Orb of Souls]})) {
			| check to see if its a Spell/AA/CombatAbility/Ability, and if not...
			/if (!(${Me.Book[${tmp_castname}]}||${Me.AltAbility[${tmp_castname}]}||${Me.CombatAbility[${tmp_castname}]}||${Me.Ability[${tmp_castname}]})) {
				
				|default to item - no way to validate an item thats on my corpse
				/if (!${FindItem[=${tmp_castname}].ID}) {

					/if (!${Bool[${Me.Inventory[Chest]}]} && !${Me.Platinum}) {
						/varset reloadOnLoot TRUE
						/varset missingSpellItem ${tmp_castname}
						/call RemoveArrayElement "${ArrayName}" "${ArrayName}[${i}]"
					} else {
						/varset reloadOnLoot TRUE
						/varset missingSpellItem ${tmp_castname}
						/bc [${${ArrayName}[${i}]}] : ${tmp_castname} i do not have this spell|aa|ability|disc|item accessible
						/beep
						/call RemoveArrayElement "${ArrayName}" "${ArrayName}[${i}]"
					}
				}
			}
		}
		
	/next i

	/declare isHealArray bool local ${Select[${ArrayName},tankHeals,importantHeals,allHeals,hotSpells,groupHeals,lifeSupport,petHeal,petHeals,lifeTaps,xtargetHeals]}
	|/echo checking array name: ${ArrayName}
	|/echo isHealArray: ${isHealArray}
	|/echo Skip Params: ${SkipParams}
	/declare isCastTargetArray bool local ${Select[${ArrayName},BotBuffs,PetBuffs,CombatBuffs,cureTargets]}
	/for i 1 to ${${ArrayName}.Size}
		| get SpellName
		/varset ${NewArrayName}[${i},${iCastName}] ${${ArrayName}[${i}].Arg[1,/]}
		| get CastTarget
		/if (${isCastTargetArray}) /varset ${NewArrayName}[${i},${iCastTarget}] ${${ArrayName}[${i}].Arg[2,/]}
		
		/if (${Bool[${Me.AltAbility[${${NewArrayName}[${i},${iCastName}]}].Spell}]}) {
			/varset ${NewArrayName}[${i},${iCastType}] AA
		} else /if (${Me.Book[${${NewArrayName}[${i},${iCastName}]}]}) {
			/varset ${NewArrayName}[${i},${iCastType}] Spell
		} else /if (${Me.CombatAbility[${${NewArrayName}[${i},${iCastName}]}]}) {
			/varset ${NewArrayName}[${i},${iCastType}] Disc
		} else /if (${Me.Ability[${${NewArrayName}[${i},${iCastName}]}]}) {
			/varset ${NewArrayName}[${i},${iCastType}] Ability			
		} else {
			/varset ${NewArrayName}[${i},${iCastType}] Item
		}
		
		|set the normal defaults
		/varset ${NewArrayName}[${i},${iSpellGem}] ${DefaultGem}
		/varset ${NewArrayName}[${i},${iMaxTries}] 5
		/varset ${NewArrayName}[${i},${iCheckFor}] -1
		/varset ${NewArrayName}[${i},${iMaxMana}] 100
		/varset ${NewArrayName}[${i},${iMinHP}] 70
		/varset	${NewArrayName}[${i},${iZone}] All
		/varset ${NewArrayName}[${i},${iIfs}] TRUE
		|don't need to set int values of 0 which are default for ints
		|/varset ${NewArrayName}[${i},${iMinEnd}] 0
		|default casting while invis to 0 which impies no casting while invis
		|/varset	${NewArrayName}[${i},${iCastInvis}] 0
		|/varset	${NewArrayName}[${i},${iMinSick}] 0
		|/varset ${NewArrayName}[${i},${iAllowSpellSwap}] 0
		|/varset ${NewArrayName}[${i},${iNoEarlyRecast}] 0
		|/varset ${NewArrayName}[${i},${iNoStack}] 0
		|/varset	${NewArrayName}[${i},${iTriggerSpell}] 0
		|default PctAggro to 0, cast even when aggro target
		|/varset	${NewArrayName}[${i},${iPctAggro}] 0


		|we have optinal params, check to see which ones we need to overwrite
		/if (!${SkipParams} && (${${ArrayName}[${i}].Find[/]} || ${isHealArray})) {

			| get SpellGem
			/if (${${ArrayName}[${i}].Find[/Gem|]}) {
				/call argueString Gem| "${${ArrayName}[${i}]}"
				/varset ${NewArrayName}[${i},${iSpellGem}] ${c_argueString}
			}
			| get SubToRun
			/if (${${ArrayName}[${i}].Find[/SubToRun|]}) {
				/call argueString SubToRun| "${${ArrayName}[${i}]}"
				/varset ${NewArrayName}[${i},${iSubToRun}] ${c_argueString}
			}
			| get GiveUpTimer
			/if (${${ArrayName}[${i}].Find[/GiveUpTimer|]}) {
				/call argueString GiveUpTimer| "${${ArrayName}[${i}]}"
				/varset ${NewArrayName}[${i},${iGiveUpTimer}] ${c_argueString}
			}
			
			| get MaxTries
			/if (${${ArrayName}[${i}].Find[/MaxTries|]}) {
				/call argueString MaxTries| "${${ArrayName}[${i}]}"
				/varset ${NewArrayName}[${i},${iMaxTries}] ${c_argueString}
			}

			| get CheckFor
			/if (${${ArrayName}[${i}].Find[/CheckFor|]}) {
				/call argueString CheckFor| "${${ArrayName}[${i}]}"
				/varset ${NewArrayName}[${i},${iCheckFor}] ${c_argueString}
			}
			
			| get MinMana
			/if (${${ArrayName}[${i}].Find[/MinMana|]}) {
				/call argueString MinMana| "${${ArrayName}[${i}]}"
				/varset ${NewArrayName}[${i},${iMinMana}] ${c_argueString}
			}
			| get MaxMana
			/if (${${ArrayName}[${i}].Find[/MaxMana|]}) {
				/call argueString MaxMana| "${${ArrayName}[${i}]}"
				/varset ${NewArrayName}[${i},${iMaxMana}] ${c_argueString}
			}

			| get MinHP
			/if (${${ArrayName}[${i}].Find[/MinHP|]}) {
				/call argueString MinHP| "${${ArrayName}[${i}]}"
				/varset ${NewArrayName}[${i},${iMinHP}] ${c_argueString}
			}

			| get HealPct
			/if (${isHealArray}) {
				
				/if (${${ArrayName}[${i}].Find[/HealPct|]}) {
					/call argueString HealPct| "${${ArrayName}[${i}]}"
					/varset ${NewArrayName}[${i},${iHealPct}] ${c_argueString}
				} else {
					/bc ${Me.Name} - ${errMsg} (Cannot find HealPct) : [${${ArrayName}[${i}]}]
					/popup ${Me.Name} - ${errMsg} (Cannot find HealPct) : [${${ArrayName}[${i}]}] 
					/beep					
					/endmacro
				}
			}		
		
			| get Reagent
			/if (${${ArrayName}[${i}].Find[/Reagent|]}) {
				/call argueString Reagent| "${${ArrayName}[${i}]}"
				/varset ${NewArrayName}[${i},${iReagent}] ${c_argueString}
			}
			
			| get NoBurn
			/if (${${ArrayName}[${i}].Find[/NoBurn]}) {
				/varset ${NewArrayName}[${i},${iNoBurn}] 1
			}
			
			| get NoAggro
			/if (${${ArrayName}[${i}].Find[/NoAggro]}) {
				/varset ${NewArrayName}[${i},${iNoAggro}] 1
			}
			
			| get Mode
			/if (${${ArrayName}[${i}].Find[/Mode|]}) {
				/call argueString Mode "${${ArrayName}[${i}]}"
				/varset ${NewArrayName}[${i},${iMode}] ${c_argueString}
			}
			
			| get Rotate
			/if (${${ArrayName}[${i}].Find[/Rotate]}) /varset ${NewArrayName}[${i},${iRotate}] 1
			

			| get Delay
			/if (${${ArrayName}[${i}].Find[/Delay|]}) {
				/call argueString Delay| "${${ArrayName}[${i}]}"
				/varset ${NewArrayName}[${i},${iDelay}] ${c_argueString}
			}
			
			| get GiftOfMana
			/if (${${ArrayName}[${i}].Find[/GoM]}) /varset ${NewArrayName}[${i},${iGiftOfMana}] 1

			| get iPctAggro
			/if (${${ArrayName}[${i}].Find[/PctAggro|]}) {
				/call argueString PctAggro| "${${ArrayName}[${i}]}"
				/varset ${NewArrayName}[${i},${iPctAggro}] ${c_argueString}
			}

			| get iZone - only for curing
			/if (${${ArrayName}[${i}].Find[/Zone|]}) {
				/call argueString Zone| "${${ArrayName}[${i}]}"
				/varset ${NewArrayName}[${i},${iZone}] ${c_argueString}
			} 
			| get iMinSick - only for curing
			/if (${${ArrayName}[${i}].Find[/MinSick|]}) {
				/call argueString MinSick| "${${ArrayName}[${i}]}"
				/varset ${NewArrayName}[${i},${iMinSick}] ${c_argueString}
			}
			
			| get iAllowSpellSwap -- used in DoT casting to allow casters to shuffle a large number of DoTs
			| DoTs defined with /AllowSpellSwap immediately de-mem after casting so another spell can be automatically memed in it's place
			/if (${${ArrayName}[${i}].Find[/AllowSpellSwap]}) {
				/varset ${NewArrayName}[${i},${iAllowSpellSwap}] 1
			}
			| get iNoEarlyRecast -- used for DoTs like Splurt/Splort that you want to wear off before recasting
			/if (${${ArrayName}[${i}].Find[/NoEarlyRecast]}) {
				/varset ${NewArrayName}[${i},${iNoEarlyRecast}] 1
			}

			| get NoStack -- used for DoTs/Debuffs that cannot be used form two people on 1 NPC at the same time. (DoTs with a debuff)
			| Note: this was created for necromancer / druid epics on Project Lazarus.
			/if (${${ArrayName}[${i}].Find[/NoStack]}) {
				/varset ${NewArrayName}[${i},${iNoStack}] 1
			}

			| get TriggerSpell -- used for DoTs only. 
			| Example: Main=Dread Pyre/TriggerSpell|Funeral Pyre of Kelador
			| Dread Pyre will cast "Dread Pyre" AND "Funeral Pyre of Keldador" at the same time (custom spells)
			| TriggerSpell check allows E3 to re-cast the spell "Dread Pyre" if either of these two DoTs are missing from the NPC.
			/if (${${ArrayName}[${i}].Find[/TriggerSpell|]}) {
				/call argueString TriggerSpell| "${${ArrayName}[${i}]}"
				/varset ${NewArrayName}[${i},${iTriggerSpell}] ${c_argueString}
			}
			

			| get iIfs - conditionals used in various places
			/if (${${ArrayName}[${i}].Find[/Ifs|]}) {
				/call argueString Ifs| "${${ArrayName}[${i}]}"
				/varset ${NewArrayName}[${i},${iIfs}] ${Ini[${Character_Ini},Ifs,${c_argueString},NULL,noparse]}
			| we kind of need to print here instead of like everything else :P
				/if (${printAll}) /echo Ifs ${c_argueString}
			}

			| get MinEnd
			/if (${${ArrayName}[${i}].Find[/MinEnd|]}) {
				/call argueString MinEnd| "${${ArrayName}[${i}]}"
				/varset ${NewArrayName}[${i},${iMinEnd}] ${c_argueString}
			}

		} 
	

		|Fill out information based off Item/AA/Spell/Disc
		/if (${${NewArrayName}[${i},${iCastType}].Equal[Item]}) {
			|ITEM INFORMATION
			|Instead of calling FindItem 12 times, we call it twice and get a direct reference to the item in question

			| ${Me.Inventory[23].Item[7].Spell.TargetType}
			/varset invSlot ${FindItem[=${${NewArrayName}[${i},${iCastName}]}].ItemSlot}
			/varset bagSlot ${FindItem[=${${NewArrayName}[${i},${iCastName}]}].ItemSlot2}

			/if (${bagSlot}==-1) {
				|/echo root item: id:${invSlot} :${${ArrayName}[${i}].Arg[1,/]}
				|Means this is not in a bag and in the root inventory
				/varset	${NewArrayName}[${i},${iTargetType}] ${Me.Inventory[${invSlot}].Spell.TargetType}
				/varset	${NewArrayName}[${i},${iDuration}] ${Me.Inventory[${invSlot}].Spell.Duration}
				/varset	${NewArrayName}[${i},${iRecastTime}] ${Me.Inventory[${invSlot}].Spell.RecastTime}
				/varset	${NewArrayName}[${i},${iRecoveryTime}] ${Me.Inventory[${invSlot}].Spell.RecoveryTime}
				/varset	${NewArrayName}[${i},${iMyCastTime}] ${Me.Inventory[${invSlot}].CastTime}
				/if (${Me.Inventory[${invSlot}].Spell.AERange} > 0) {
					/varset	${NewArrayName}[${i},${iMyRange}] ${Me.Inventory[${invSlot}].Spell.AERange}
				} else {
					/varset	${NewArrayName}[${i},${iMyRange}] ${Me.Inventory[${invSlot}].Spell.MyRange}
				}
				/if (${Me.Inventory[${invSlot}].EffectType.Equal[Click Worn]}) {
					/varset	${NewArrayName}[${i},${iItemMustEquip}] ${Me.Inventory[${invSlot}].WornSlot[1].Name}
				}
				/varset	${NewArrayName}[${i},${iSpellName}] ${Me.Inventory[${invSlot}].Spell}
				/varset	${NewArrayName}[${i},${iSpellID}] ${Me.Inventory[${invSlot}].Spell.ID}
				/varset	${NewArrayName}[${i},${iCastID}] ${Me.Inventory[${invSlot}].ID}
				/varset	${NewArrayName}[${i},${iSpellType}] ${Me.Inventory[${invSlot}].Spell.SpellType}

			} else {
				|1 index vs 0 index
				/varcalc bagSlot ${bagSlot}+1
				|/echo bag item:id id:${invSlot},${bagSlot}:${${ArrayName}[${i}].Arg[1,/]}
				/varset	${NewArrayName}[${i},${iTargetType}] ${Me.Inventory[${invSlot}].Item[${bagSlot}].Spell.TargetType}
				/varset	${NewArrayName}[${i},${iDuration}] ${Me.Inventory[${invSlot}].Item[${bagSlot}].Spell.Duration}
				/varset	${NewArrayName}[${i},${iRecastTime}] ${Me.Inventory[${invSlot}].Item[${bagSlot}].Spell.RecastTime}
				/varset	${NewArrayName}[${i},${iRecoveryTime}] ${Me.Inventory[${invSlot}].Item[${bagSlot}].Spell.RecoveryTime}
				/varset	${NewArrayName}[${i},${iMyCastTime}] ${Me.Inventory[${invSlot}].Item[${bagSlot}].CastTime}
				/if (${Me.Inventory[${invSlot}].Item[${bagSlot}].Spell.AERange} > 0) {
					/varset	${NewArrayName}[${i},${iMyRange}] ${Me.Inventory[${invSlot}].Item[${bagSlot}].Spell.AERange}
				} else {
					/varset	${NewArrayName}[${i},${iMyRange}] ${Me.Inventory[${invSlot}].Item[${bagSlot}].Spell.MyRange}
				}
				/if (${Me.Inventory[${invSlot}].Item[${bagSlot}].EffectType.Equal[Click Worn]}) {
					/varset	${NewArrayName}[${i},${iItemMustEquip}] ${Me.Inventory[${invSlot}].Item[${bagSlot}].WornSlot[1].Name}
				}
				/varset	${NewArrayName}[${i},${iSpellName}] ${Me.Inventory[${invSlot}].Item[${bagSlot}].Spell}
				/varset	${NewArrayName}[${i},${iSpellID}] ${Me.Inventory[${invSlot}].Item[${bagSlot}].Spell.ID}
				/varset	${NewArrayName}[${i},${iCastID}] ${Me.Inventory[${invSlot}].Item[${bagSlot}].ID}
				/varset	${NewArrayName}[${i},${iSpellType}] ${Me.Inventory[${invSlot}].Item[${bagSlot}].Spell.SpellType}
			}

		} else /if (${${NewArrayName}[${i},${iCastType}].Equal[AA]}) {
			|AA INFORMATION
			/varset	${NewArrayName}[${i},${iTargetType}] ${Me.AltAbility[${${NewArrayName}[${i},${iCastName}]}].Spell.TargetType}
			/varset	${NewArrayName}[${i},${iDuration}] ${Me.AltAbility[${${NewArrayName}[${i},${iCastName}]}].Spell.Duration}
			/varset	${NewArrayName}[${i},${iRecastTime}] ${Me.AltAbility[${${NewArrayName}[${i},${iCastName}]}].ReuseTime}
			/varset	${NewArrayName}[${i},${iRecoveryTime}] ${Me.AltAbility[${${NewArrayName}[${i},${iCastName}]}].Spell.RecoveryTime}
			/varset	${NewArrayName}[${i},${iMyCastTime}] ${Me.AltAbility[${${NewArrayName}[${i},${iCastName}]}].Spell.MyCastTime}
			/if (${Me.AltAbility[${${NewArrayName}[${i},${iCastName}]}].Spell.AERange} > 0) {
				/varset	${NewArrayName}[${i},${iMyRange}] ${Me.AltAbility[${${NewArrayName}[${i},${iCastName}]}].Spell.AERange}
			} else {
				/varset	${NewArrayName}[${i},${iMyRange}] ${Me.AltAbility[${${NewArrayName}[${i},${iCastName}]}].Spell.MyRange}
			}
			/varset	${NewArrayName}[${i},${iSpellName}] ${Me.AltAbility[${${NewArrayName}[${i},${iCastName}]}].Spell}
			/varset	${NewArrayName}[${i},${iSpellID}] ${Me.AltAbility[${${NewArrayName}[${i},${iCastName}]}].Spell.ID}
			/varset	${NewArrayName}[${i},${iCastID}] ${Me.AltAbility[${${NewArrayName}[${i},${iCastName}]}].ID}
			/varset	${NewArrayName}[${i},${iSpellType}] ${Me.AltAbility[${${NewArrayName}[${i},${iCastName}]}].Spell.SpellType}	
		
		} else /if (${${NewArrayName}[${i},${iCastType}].Equal[Spell]}) {
			|SPELL INFORMATION
			/varset	${NewArrayName}[${i},${iTargetType}] ${Spell[${${NewArrayName}[${i},${iCastName}]}].TargetType}
			/varset	${NewArrayName}[${i},${iDuration}] ${Spell[${${NewArrayName}[${i},${iCastName}]}].Duration}	
			/varset	${NewArrayName}[${i},${iRecastTime}] ${Spell[${${NewArrayName}[${i},${iCastName}]}].RecastTime}	
			/varset	${NewArrayName}[${i},${iRecoveryTime}] ${Spell[${${NewArrayName}[${i},${iCastName}]}].RecoveryTime}	
			/varset	${NewArrayName}[${i},${iMyCastTime}] ${Spell[${${NewArrayName}[${i},${iCastName}]}].MyCastTime}	
			/if (${Spell[${${NewArrayName}[${i},${iCastName}]}].AERange} > 0 && ${Spell[${${NewArrayName}[${i},${iCastName}]}].MyRange} == 0) {
				/varset	${NewArrayName}[${i},${iMyRange}] ${Spell[${${NewArrayName}[${i},${iCastName}]}].AERange}
			} else {
				/varset	${NewArrayName}[${i},${iMyRange}] ${Spell[${${NewArrayName}[${i},${iCastName}]}].MyRange}
			}			
			/varset ${NewArrayName}[${i},${iMana}] ${Spell[${${NewArrayName}[${i},${iCastName}]}].Mana}
			/varset	${NewArrayName}[${i},${iSpellName}] ${Spell[${${NewArrayName}[${i},${iCastName}]}]}
			/varset	${NewArrayName}[${i},${iSpellID}] ${Spell[${${NewArrayName}[${i},${iCastName}]}].ID}
			/varset	${NewArrayName}[${i},${iCastID}] ${Spell[${${NewArrayName}[${i},${iCastName}]}].ID}
			/varset	${NewArrayName}[${i},${iSpellType}] ${Spell[${${NewArrayName}[${i},${iCastName}]}].SpellType}
		
		} else /if (${${NewArrayName}[${i},${iCastType}].Equal[Disc]}) {
			|DISC INFORMATION
			/varset	${NewArrayName}[${i},${iDuration}] ${Spell[${${NewArrayName}[${i},${iCastName}]}].Duration}
			/if (${Spell[${${NewArrayName}[${i},${iCastName}]}].AERange} > 0) {
				/varset	${NewArrayName}[${i},${iMyRange}] ${Spell[${${NewArrayName}[${i},${iCastName}]}].AERange}
			} else {
				/varset	${NewArrayName}[${i},${iMyRange}] ${Spell[${${NewArrayName}[${i},${iCastName}]}].MyRange}
			}
			/varset	${NewArrayName}[${i},${iSpellName}] ${Spell[${${NewArrayName}[${i},${iCastName}]}]}
			/varset	${NewArrayName}[${i},${iSpellID}] ${Spell[${${NewArrayName}[${i},${iCastName}]}].ID}
			/varset	${NewArrayName}[${i},${iCastID}] ${Spell[${${NewArrayName}[${i},${iCastName}]}].ID}
		
		
		}

		| get CheckForID
		/if (${Bool[${AltAbility[${${NewArrayName}[${i},${iCheckFor}]}].Spell}]}) {
			/varset ${NewArrayName}[${i},${iCheckForID}] ${AltAbility[${${NewArrayName}[${i},${iCheckFor}]}].Spell.ID}
		} else /if (${Bool[${Spell[${${NewArrayName}[${i},${iCheckFor}]}].ID}]}) {
			/varset ${NewArrayName}[${i},${iCheckForID}] ${Spell[${${NewArrayName}[${i},${iCheckFor}]}].ID}
		} else {
			/varset ${NewArrayName}[${i},${iCheckForID}] -1
		}

	
		|Print out debug information if requsted
		/if (${printAll} || ${printMin} || ${isHealArray}) {
	
			/if (${printAll} || ${printMin}) /echo CastName ${${NewArrayName}[${i},${iCastName}]}
			/if (${printAll}  || ${printMin}) /echo CastType ${${NewArrayName}[${i},${iCastType}]}
			/if (${printAll}) /echo TargetType ${${NewArrayName}[${i},${iTargetType}]}
			/if (${printAll}|| ${printMin}) /echo SpellGem ${${NewArrayName}[${i},${iSpellGem}]}
			/if (${printAll}|| ${printMin}) /echo HealPct ${${NewArrayName}[${i},${iHealPct}]}
			/if (${printAll}) /echo SubToRun ${${NewArrayName}[${i},${iSubToRun}]}
			/if (${printAll}) /echo GiveUpTimer ${${NewArrayName}[${i},${iGiveUpTimer}]}
			/if (${printAll}) /echo MaxTries ${${NewArrayName}[${i},${iMaxTries}]}
			/if (${printAll}) /echo CheckFor ${${NewArrayName}[${i},${iCheckFor}]}
			/if (${printAll}) /echo MinMana ${${NewArrayName}[${i},${iMinMana}]}
			/if (${printAll}) /echo MaxMana ${${NewArrayName}[${i},${iMaxMana}]}
			/if (${printAll}) /echo MinHP ${${NewArrayName}[${i},${iMinHP}]}
			/if (${printAll}) /echo Reagent ${${NewArrayName}[${i},${iReagent}]}
			/if (${printAll}) /echo NoBurn ${${NewArrayName}[${i},${iNoBurn}]}
			/if (${printAll}) /echo NoAggro ${${NewArrayName}[${i},${iNoAggro}]}
			/if (${printAll}) /echo Mode ${${NewArrayName}[${i},${iMode}]}	
			/if (${printAll}) /echo Rotate ${${NewArrayName}[${i},${iRotate}]}	
			/if (${printAll}) /echo Delay ${${NewArrayName}[${i},${iDelay}]}	
			/if (${printAll}) /echo GiftOfMana ${${NewArrayName}[${i},${iGiftOfMana}]}
			/if (${printAll}) /echo PctAggro ${${NewArrayName}[${i},${iPctAggro}]}
			/if (${printAll}) /echo Zone ${${NewArrayName}[${i},${iZone}]}
			/if (${printAll}) /echo Zone ${${NewArrayName}[${i},${iMinSick}]}
			/if (${printAll}) /echo AllowSpellSwap ${${NewArrayName}[${i},${iAllowSpellSwap}]}	
			/if (${printAll}) /echo NoEarlyRecast ${${NewArrayName}[${i},${iNoEarlyRecast}]}
			/if (${printAll}) /echo NoStack ${${NewArrayName}[${i},${iNoStack}]}
			/if (${printAll}) /echo TriggerSpell ${${NewArrayName}[${i},${iTriggerSpell}]}
			|This print is an oddball, had to leave it back in the setting of the variable
			|as it does some argument parsing to get it and the result is
			/if (${printAll}) {
				/if (${${NewArrayName}[${i},${iIfs}]}) {
					/echo Ifs always TRUE
				} 	
			}
			/if (${printAll}) /echo MinEnd ${${NewArrayName}[${i},${iMinEnd}]}
			/if (${printAll}  || ${printMin}) /echo SpellDuration ${${NewArrayName}[${i},${iDuration}]}
			/if (${printAll}|| ${printMin}) /echo RecastTime ${${NewArrayName}[${i},${iRecastTime}]}
			/if (${printAll}) /echo RecoveryTime ${${NewArrayName}[${i},${iRecoveryTime}]}
			/if (${printAll} || ${printMin}) /echo ${NewArrayName} ${i}  MyCastTime ${${NewArrayName}[${i},${iMyCastTime}]}
			/if (${printAll}) /echo MyRange ${${NewArrayName}[${i},${iMyRange}]}
			/if (${printAll}) /echo Mana ${${NewArrayName}[${i},${iMana}]}
			/if (${printAll}) /echo ItemMustEquip ${${NewArrayName}[${i},${iItemMustEquip}]}
			/if (${printAll}) /echo SpellName ${${NewArrayName}[${i},${iSpellName}]}
			/if (${printAll}) /echo SpellID ${${NewArrayName}[${i},${iSpellID}]}
			/if (${printAll}) /echo iCastID ${${NewArrayName}[${i},${iCastID}]}
			/if (${printAll}) /echo ${${NewArrayName}[${i},${iCastName}]} ${${NewArrayName}[${i},${iCheckFor}]} iCheckForID ${${NewArrayName}[${i},${iCheckForID}]}
			/if (${printAll}) /echo SpellType ${${NewArrayName}[${i},${iSpellType}]}
			/if (${printAll}) /echo CastTarget ${${NewArrayName}[${i},${iCastTarget}]}
			/if (${printAll}) /echo ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
								
		}
		
	/next i
	|/deletevar ${ArrayName}
/RETURN	${NewArrayName}

|------------------------------------------------|
|- Removes elements from a 2D array.			      -|
SUB RemoveSpellArrayElement(ArrayName, ArrayIndex)
/if (${Debug}) 	/g |- RemoveSpellArrayElement ==> Removing [${ArrayIndex}] from [${ArrayName}] ${Time}
	/declare o int local
	/declare n int local 1
	/declare p int local
	/declare tempArraySize int local
	| only do this if the array has more than one entry to prevent blocking
	| copy to temp array, excluding the removed index
	| delete original array
	| create original array name, copy contents from tempArray, delete temp array
	/if (${${ArrayName}.Size[1]}>1) {
		/varcalc tempArraySize ${${ArrayName}.Size[1]}-1
		/declare tempArray[${tempArraySize},${SpellProp.Size}]
		/for o 1 to ${${ArrayName}.Size[1]}	
			/if (${o} != ${ArrayIndex}) {
				/for p 1 to ${SpellProp.Size}
				  |/echo tempArray[${n},${p}] ${${ArrayName}[${o},${p}]} ${SpellProp[${p}]}
					/varset tempArray[${n},${p}] ${${ArrayName}[${o},${p}]}
          |/echo tempArray[${n},${p}] ${${ArrayName}[${o},${p}]} ${SpellProp[${p}]}
				/next p
				/varcalc n ${n} + 1
			}
		/next o
		| Delete the existing array.
		/deletevar ${ArrayName}
		/declare ${ArrayName}[${tempArraySize},${SpellProp.Size}] string outer
		
		/for n 1 to ${tempArraySize}
			/for p 1 to ${SpellProp.Size}
				/varset ${ArrayName}[${n},${p}] ${tempArray[${n},${p}]}
      | /echo ${ArrayName}[${n},${p}] ${${ArrayName}[${n},${p}]} ${SpellProp[${p}]}
			/next p
		/next n
		/deletevar tempArray
	} else {
	  /deletevar ${ArrayName}
	}
/if (${Debug}) /g <== RemoveSpellArrayElement -| ${Time}
/RETURN

|----------------------------------------------------|
|- Removes elements from an existing array.			-|
|----------------------------------------------------|
|- @param ArrayName: Name of the array to search.	-|
|- @param Data: Data you wish to remove.			-|
|----------------------------------------------------|
SUB RemoveArrayElement(ArrayName, Data)
/if (${Debug}) {
	/echo |- RemoveArrayElement ==>
	/echo |- RemoveArrayElement -| Removing [${Data}] from [${ArrayName}]
}
	/declare remainingElements string local
	/declare newArraySize int local
	/declare i int local
	| Count array elements that do not match 'Data'.
	/for i 1 to ${${ArrayName}.Size}
		/if (${${ArrayName}[${i}].NotEqual[${Data}]}) {
			/varset remainingElements ${remainingElements}${${ArrayName}[${i}]},
			/varcalc newArraySize ${newArraySize} + 1
		}
	/next i
	| If there are  elements remaining in the array, create a copy of the new array
	/if (${newArraySize}) {
		/declare newArray[${newArraySize}] string local
		/for i 1 to ${newArraySize}
			/varset newArray[${i}] ${remainingElements.Arg[${i},,]}
		/next i
	}
	| Delete the existing array.
	/deletevar ${ArrayName}
	| If there is a new array to make
	/if (${Defined[newArray]}) {
		/declare ${ArrayName}[${newArray.Size}] string outer
		/for i 1 to ${newArray.Size}
			/varset ${ArrayName}[${i}] ${newArray[${i}]}
		/next i
	}
/if (${Debug}) /echo <== RemoveArrayElement -|
/RETURN
|----------------------------------------------------|
SUB RemoveArrayElements(ArrayName, ArrayIndexes)
  /declare i int local
  /declare n int local 0
  /declare tempArray[${${ArrayName}.Size}] string local 0

  /for i 1 to ${${ArrayName}.Size}
    |/echo a ${${ArrayName}[${i}]} f ${Bool[${ArrayIndexes.Find[${i},]}]}
    /if (!${Bool[${ArrayIndexes.Find[${i},]}]}) {
      /varcalc n ${n}+1
      /varset tempArray[${n}] ${${ArrayName}[${i}]}
    }
  /next i
  /deletevar ${ArrayName}
  /if (${n}>0) {
    /declare ${ArrayName}[${n}] string outer
    /for i 1 to ${n}
      |/echo ${i} ${n} n ${ArrayName}[${i}] t tempArray[${i}]
      /varset ${ArrayName}[${i}] ${tempArray[${i}]}
    /next i
  }
  /deletevar tempArray
/RETURN
|------------------------------------------------------------|
|- Reliably targets specified Target.IDs.					-|
|------------------------------------------------------------|
|- @param TargetID: The ID of the spawn you wish to target.	-|
|------------------------------------------------------------|
SUB TrueTarget(TargetID)
	| Check if the target is in zone.
	/if (${SpawnCount[id ${TargetID}]}) {
			/squelch /target id ${TargetID}
			/delay 3s ${Target.ID} == ${TargetID}
	} else {
		/echo TrueTarget has no spawncount
	}
/RETURN
|------------------------------------------------------------------------------------|
|- Creates timers, at the specified duration, and adds their names to timerArray.	-|
|------------------------------------------------------------------------------------|
|- @param timerName: Name of the timer you want to declare.							-|
|- @param timerSetting: Setting of the timer you're declaring.						-|
|------------------------------------------------------------------------------------|
SUB createTimer(timerName, timerSetting)
/if (${Debug}) /echo |- createTimer ==>
  /if (${timerName.Length} == 0) /return
  /if (${Defined[${timerName}]}) {
    /varset ${timerName} ${timerSetting}
  } else {
    /declare ${timerName} timer outer ${timerSetting}
    /call BuildArray timerArray ${timerName}
  }
/if (${Debug}) /echo <== createTimer -| created ${timerName} [${${timerName}}]
/RETURN
|----------------------------------------------------|
|- Deletes spent timers made by 'SUB createTimer'.	-|
|----------------------------------------------------|
SUB check_macroTimers
/if (${Debug}) /echo |- check_macroTimers ==>
	/declare i int local
	/declare SpentTimerCounter int local
	/declare SpentTimerList string local
	/for i 1 to ${timerArray.Size}
    /if (${Defined[${timerArray[${i}]}]}) {
		  /if (!${${timerArray[${i}]}}) {
        /if (${Debug}) /echo Deleting timer: ${timerArray[${i}]}
			    /varcalc SpentTimerCounter ${SpentTimerCounter} + 1
				/deletevar ${timerArray[${i}]}
				/varset SpentTimerList ${SpentTimerList}${i},
			}
		}
	/next i
	| If all the timers listed on timerArray are spent, delete timerArray.
	/if (${SpentTimerCounter}==${timerArray.Size}) {
		/if (${Debug}) /echo All timers have been cleared.
		/deletevar timerArray
	} else {
    /call RemoveArrayElements "timerArray" ${SpentTimerList}"
	}
/if (${Debug}) /echo <== check_macroTimers -|
/RETURN
|--------------------------------------------------------------------|
|- Deletes spell resist counters thare are defined during combat	-|
|--------------------------------------------------------------------|
SUB check_ResistCounters


   /if (!${Defined[spellResistCounters]}) {
        /declare spellResistCounters[1024] string outer
        /declare spellResistCountersIndex int outer
        /varset spellResistCountersIndex 0
  } 
  |we are no longer in combat, so we can now clear all the values in the array and set the current index to 0
  /if (!${Defined[DoT_Targets]} && ${spellResistCountersIndex} > 0) {

      /declare s int local
      /declare nameOfCounter string local
      /for s 1 to ${spellResistCountersIndex}
      
        /varset nameOfCounter ${spellResistCounters[${s}]}
	    /if (${Debug}) /echo check_ResistCounterReset checking for defined ${nameOfCounter}
		/if (${Defined[${nameOfCounter}]}) {
		  /if (${Debug}) /echo check_ResistCounterReset deleting defined ${nameOfCounter}
         /deletevar ${nameOfCounter}
        } else {
			/if (${Debug}) /echo check_ResistCounterReset there nothing defined for  ${nameOfCounter}
		}
      
      /next s
    
      /varset spellResistCountersIndex 0
  }

/Return
SUB register_ResistCounter(resistVariableName)
	/if (!${Defined[spellResistCounters]}) {
			/declare spellResistCounters[1024] string outer
			/declare spellResistCountersIndex int outer
			/varset spellResistCountersIndex 0
	} 
	/if (${Debug}) /echo register_ResistCounter registering ${resistVariableName}
	/declare ${resistCount} int outer 1
	/varcalc spellResistCountersIndex ${spellResistCountersIndex}+1
	/if (${Debug}) /echo spellResistCountersIndex is now:${spellResistCountersIndex}
 	/varset spellResistCounters[${spellResistCountersIndex}] ${resistVariableName}


/Return
|----------------------------------------------------------------------------------------|
|- Auto inventories items from your cursor. 											-|
|- Alerts the user, and ends the macro, if there is no free inventory space available.	-|
|----------------------------------------------------------------------------------------|
SUB ClearCursor
/if (${Debug}) /echo |- ClearCursor ==>
	/declare i int local 1
  :check_cursor
	/if (${Cursor.ID}) {
	/bc AutoInventory Item: ${Cursor} 
    /autoinventory
    /if (${i} > 5) {
      /beep
      /bc ${Me.Name} cannot clear cursor, ending macro
      /popup ${Me.Name} cannot clear cursor, ending macro
      /endmacro
    } else {
      /delay 1s !${Cursor.ID}
    }
    /varcalc i ${i}+1
    /goto :check_cursor
  }
/if (${Debug}) /echo <== ClearCursor -|
/RETURN
|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Sub SwapItem(itemName,slotName)
  /if (!${Defined[itemName]} || !${Defined[slotName]}) /return
  /if (${Cursor.ID}) /call ClearCursor
	|if the item is in a bag rather than top level inventory slot, open the bag prior to exchange
	/if (${Me.Inventory[${FindItem[=${itemName}].ItemSlot}].Container}) {
    /declare packSlot int local ${Math.Calc[${FindItem[=${itemName}].ItemSlot}-22]}
    /if (!${Window[Pack${packSlot}].Open}) /itemnotify Pack${packSlot} rightmouseup
    /delay 2s ${Window[Pack${packSlot}].Open}
	}
  /delay 1
  |/bc exchanging "${itemName}" ${slotName}
  /if (${slotName.Equal[Cursor]}) {
		/itemnotify "${itemName}" leftmouseup
		/delay 1s ${Cursor.Name.Equal[${itemName}]}
  } else {
		/exchange "${itemName}" "${slotName}"
		|/echo attempted exchange
		/delay 5s ${Me.Inventory[${slotName}].Name.Equal[${itemName}]}
    /delay 1s !${Cursor.ID}
    /if (${Cursor.ID}) /call ClearCursor
  }
  /if (${Defined[packSlot]}) {
    /if (${Window[Pack${packSlot}].Open}) /itemnotify Pack${packSlot} rightmouseup
    /delay 2s !${Window[Pack${packSlot}].Open}
  }
/return
|-------------------------------------------------------------|
|- Use MQ2MoveUtils /moveto function, to move to destination.	||
SUB MoveToLoc(yLoc,xLoc,timeToWait,arrivalDist)
  /if (!${Bool[${arrivalDist}]}) /varset arrivalDist 10
  /if (!${Bool[${timeToWait}]}) /varset timeToWait 30
  /declare moveTimer timer local ${timeToWait}
  |/echo MoveToLoc ${yLoc},${xLoc},${timeToWait},${arrivalDist}
  /if (${Math.Distance[${yLoc},${xLoc}]}<=${arrivalDist}) /return
  /if (${Me.Casting.ID} && ${Me.Class.ShortName.NotEqual[BRD]}) /call interrupt
  /squelch /moveto loc ${yLoc} ${xLoc} dist ${arrivalDist}
  :retryMove
  /call Background_Events
  /delay ${Math.Calc[${timeToWait}/5]} ${Math.Distance[${yLoc},${xLoc}]} <=${arrivalDist}
  /if (${Math.Distance[${yLoc},${xLoc}]}<=${arrivalDist}) {
    /moveto off
    /return
  } else /if (${moveTimer}) {
  	/goto :retryMove
	} else /moveto off
/return
|--------------------------------------------------------------------------|
|- Type Stuff
|- Lifted from commonsubs.inc by Armysoldier
Sub Type(InStr)
  /declare char string local
  /declare loopctr int local
  /for loopctr 1 to ${InStr.Length}
  /varset char ${InStr.Mid[${loopctr},1]}
  /if (!${char.Length}) {
    /nomodkey /keypress space chat
  } else {
    /nomodkey /keypress ${char} chat
  }
  /next loopctr
/return

|--------------------------------------------------------------------------|
Sub RemoveComma(InStr)
  /declare newStr string local
  /declare i int local
  /for i 1 to 20
|/echo ${i} ${Bool[${InStr.Arg[${i},,]}]} ${newStr} ${InStr.Arg[${i},,]}
  /if (${Bool[${InStr.Arg[${i},,]}]}) {
    /varset newStr ${newStr} ${InStr.Arg[${i},,]}
  } else {
    /return ${newStr}
  }
  /next i
/return ${newStr}

|--------------------------------------------------------------------------|
SUB argueString(WhatToFind, givenData)
	/declare i int local 1
	/varset c_argueString NULL
	:nextArg
	/if (${givenData.Find[/]}) {		
		/if (${givenData.Arg[${i},/].Find[${WhatToFind}]}) {		
			/varset c_argueString ${givenData.Arg[${i},/].Arg[2,|]}
		} else {
			/varcalc i ${i} + 1
			/if (${givenData.Arg[${i},/].Length}) /goto :nextArg
		}
	} else {
		/if (${givenData.Arg[${i}, ].Find[${WhatToFind}]}) {		
			/varset c_argueString ${givenData.Arg[${i}, ].Arg[2,|]}
		} else {
			/varcalc i ${i} + 1
			/if (${givenData.Arg[${i}, ].Length}) /goto :nextArg
		}
	}
/RETURN
|--------------------------------------------------------------------------|
Sub check_selectedBots(selectedBots, ChatSender)
  /declare botSelected bool local FALSE
  /if (${selectedBots.Find[ ]}) /varset selectedBots ${selectedBots.Replace[ ,,]}
  /if (${Select[me,${selectedBots}]} && ${ChatSender.Equal[${Me.Name}]}) /varset botSelected TRUE
  /if (${Select[${Me.Class.ShortName},${selectedBots}]}) /varset botSelected TRUE
  /if (${Select[${Me.CleanName},${selectedBots}]}) /varset botSelected TRUE
  /if (${Select[melee,${selectedBots}]} && ${Select[${Me.Class.ShortName},BRD,BER,BST,MNK,PAL,RNG,ROG,SHD,WAR]}) /varset botSelected TRUE
  /if (${Select[casters,${selectedBots}]} && ${Select[${Me.Class.ShortName},ENC,MAG,NEC,WIZ]}) /varset botSelected TRUE
  /if (${Select[hybrids,${selectedBots}]} && ${Select[${Me.Class.ShortName},PAL,SHD,RNG,BST]}) /varset botSelected TRUE
  /if (${Select[knights,${selectedBots}]} && ${Select[${Me.Class.ShortName},PAL,SHD]}) /varset botSelected TRUE
  /if (${Select[tanks,${selectedBots}]} && ${Select[${Me.Class.ShortName},PAL,SHD,WAR]}) /varset botSelected TRUE
  /if ((${Select[priests,${selectedBots}]} || ${Select[healers,${selectedBots}]}) && ${Select[${Me.Class.ShortName},CLR,DRU,SHM]}) /varset botSelected TRUE
  /if (${Select[plate,${selectedBots}]} && ${Select[${Me.Class.ShortName},WAR,BRD,CLR,PAL,SHD]}) /varset botSelected TRUE
  /if (${Select[leather,${selectedBots}]} && ${Select[${Me.Class.ShortName},DRU,BST,MNK]}) /varset botSelected TRUE
  /if (${Select[chain,${selectedBots}]} && ${Select[${Me.Class.ShortName},ROG,BER,SHM,RNG]}) /varset botSelected TRUE
  /if (${Select[silk,${selectedBots}]} && ${Select[${Me.Class.ShortName},ENC,MAG,NEC,WIZ]}) /varset botSelected TRUE
  |/if (${Select[all,${selectedBots}]} || ${Select[${Me.Class},${selectedBots}]} || ${Select[${Me.Class.ShortName},${selectedBots}]} || ${Select[${Me.CleanName},${selectedBots}]}) /varset botSelected TRUE
  /if (${Select[group,${selectedBots}]} && ${NetBots[${ChatSender}].InGroup}) /varset botSelected TRUE
  /if (${Select[raid,${selectedBots}]} && ${Raid.Members} > 0 && ${Bool[${Raid.Member[${ChatSender}]}]}) /varset botSelected TRUE
  /if (${Select[zone,${selectedBots}]} && ${NetBots[${ChatSender}].InZone}) /varset botSelected TRUE
  /if (${Select[leaders,${selectedBots}]} && ${Group}) /if (${Group.Leader.Name.Equal[${Me.Name}]}) /varset botSelected TRUE
  |/echo csb ${selectedBots}-${ChatSender}-${botSelected}
/return ${botSelected}
|--------------------------------------------------------------------------|
SUB checkEventArgs(ChatSender, eventLine, UZR, ArgData)
  |/gu CEA ChatSender ${ChatSender} | el ${eventLine} | uz ${UZR} | ad ${ArgData}
  /varset c_eventArg FALSE
  /varset c_eventArgData
  /varset c_eventArgChatSender
  /declare userValidated bool local TRUE
  /declare includeBot bool local TRUE
  /declare inZone bool local TRUE
  /declare inRange bool local TRUE
  |-Parse ChatSender.
  /if (${ChatSender.Left[1].Compare[ ]}<0)  /varset ChatSender ${ChatSender.Right[-2].Left[-1]}
  /if (${ChatSender.Equal[You]})            /varset ChatSender ${Me.CleanName}
  /if (${ChatSender.Equal[MQ2]})            /varset ChatSender ${Me.CleanName}
  /if (${ChatSender.Right[5].Equal[(msg)]}) /varset ChatSender ${ChatSender.Left[-5]}

  /if (${UZR.Find[U]} && !${NetBots[${ChatSender}].ID})     /varset userValidated FALSE
  /if (${UZR.Find[Z]} && !${NetBots[${ChatSender}].InZone}) /varset inZone FALSE
  /if (${UZR.Find[R]} && (${Spawn[pc =${ChatSender}].Distance}>${LeashLength} || ${Spawn[pc =${ChatSender}].DistanceZ}>150)) /varset inRange FALSE
  |/echo ${eventLine.Right[1].Equal[']} ${eventLine} ${eventLine.Left[-1]}
  /if (${eventLine.Right[1].Equal[']}) /varset eventLine ${eventLine.Left[-1]}
  /if (${eventLine.Find[/only|]}) {
    /varset includeBot FALSE
    /call argueString only| "${eventLine}"
    /if (${check_selectedBots[${c_argueString},${ChatSender}]}) /varset includeBot TRUE
  }
  /if (${eventLine.Find[/exclude|]}) {
    /call argueString exclude| "${eventLine}"
    /if (${check_selectedBots[${c_argueString},${ChatSender}]}) /varset includeBot FALSE
  }
  /if (${eventLine.Find[/not|]}) {
    /call argueString not| "${eventLine}"
    /if (${check_selectedBots[${c_argueString},${ChatSender}]}) /varset includeBot FALSE
  }
  /if (${eventLine.Find[/include|]}) {
    /call argueString include| "${eventLine}"
    /if (${check_selectedBots[${c_argueString},${ChatSender}]}) /varset includeBot TRUE
  }
  |/echo ${userValidated} && ${includeBot} && ${inRange} && ${inZone}
  /if (${userValidated} && ${includeBot} && ${inRange} && ${inZone}) /varset c_eventArg TRUE
  /varset c_eventArgChatSender ${ChatSender}
  /if (${Bool[${ArgData}]}) {
    /if (${ArgData.Find[/]}) {
      /varset c_eventArgData ${ArgData.Left[${Math.Calc[${ArgData.Find[/]}-2]}]}
    } else {
      /varset c_eventArgData ${ArgData}
    }
  }
  |/echo cs ${ChatSender} ad ${ArgData} find ${ArgData.Find[/]} erd ${c_eventArgData}
/RETURN ${c_eventArg}

|----------------------------------------------------------------------------|
#EVENT macroHelp "[MQ2] Help"
#EVENT macroHelp "<#*#> Help"
#EVENT macroHelp "[MQ2] Help #1#"
#EVENT macroHelp "<#*#> Help #1#"
SUB EVENT_macroHelp(line, chapter)
/if (${Debug}) /echo |- EVENT_macroHelp ==>

	/declare help_Ini string local e3 Includes\e3 Help.ini
	/declare i int local 1
	/declare e int local 1
	
	/if (!${Defined[chapter]}) {
		
		/echo Welcome to e3's in game help function.  Please choose a topic you'd like to know more about.

		/echo ----------------------------------------
		:next_Chapter
		/if (${Ini[${help_Ini}].Arg[${i},|].Length}) {
			
			/if (${Ini[${help_Ini}].Arg[${i},|].NotEqual[macroResponses]}) /echo [${Ini[${help_Ini}].Arg[${i},|]}]
			/varcalc i ${i} + 1
			/goto :next_Chapter
		}
	} else {
	
		| if the entry is not found
		/if (!${Ini[${help_Ini},${chapter}].Length}) {
			/echo Could not find an entry for [${chapter}].
		} else {
		
			:next_Topic
			/if (${Ini[${help_Ini},${chapter}].Arg[${i},|].Length}) {
				
				/echo ----------------------------------------
				/echo    ( ${Ini[${help_Ini},${chapter}].Arg[${i},|]} )
				/if (${Ini[${help_Ini},${chapter},${Ini[${help_Ini},${chapter}].Arg[${i},|]}].Length}) {
	
					/varset e 1
					:next_helpEcho
					/if (${Ini[${help_Ini},${chapter},${Ini[${help_Ini},${chapter}].Arg[${i},|]}].Arg[${e},^].Length}) {
						/echo ${Ini[${help_Ini},${chapter},${Ini[${help_Ini},${chapter}].Arg[${i},|]}].Arg[${e},^]}
						
						/varcalc e ${e} + 1
						/goto :next_helpEcho
					}
				}

				/varcalc i ${i} + 1
				/goto :next_Topic
			}
			/echo ----------------------------------------
		}
	}
	
/if (${Debug}) /echo <== EVENT_macroHelp -|
/RETURN

|----------------------------------------------------------------------------|
|- Build an ini file, of a given name, that contains alphabetized sections.	-|
|----------------------------------------------------------------------------|
|- @param Ini_File: Name of the ini file you want to create.				-|
|----------------------------------------------------------------------------|
SUB Build_Alphabetized_Ini(Ini_File)
/if (${Debug}) /echo |- Build_Alphabetized_Ini ==>
	/ini "${Ini_File}" A "A is for"
	/ini "${Ini_File}" B "B is for"
	/ini "${Ini_File}" C "C is for"
	/ini "${Ini_File}" D "D is for"
	/ini "${Ini_File}" E "E is for"
	/ini "${Ini_File}" F "F is for"
	/ini "${Ini_File}" G "G is for"
	/ini "${Ini_File}" H "H is for"
	/ini "${Ini_File}" I "I is for"
	/ini "${Ini_File}" J "J is for"
	/ini "${Ini_File}" K "K is for"
	/ini "${Ini_File}" L "L is for"
	/ini "${Ini_File}" M "M is for"
	/ini "${Ini_File}" N "N is for"
	/ini "${Ini_File}" O "O is for"
	/ini "${Ini_File}" P "P is for"
	/ini "${Ini_File}" Q "Q is for"
	/ini "${Ini_File}" R "R is for"
	/ini "${Ini_File}" S "S is for"
	/ini "${Ini_File}" T "T is for"
	/ini "${Ini_File}" U "U is for"
	/ini "${Ini_File}" V "V is for"
	/ini "${Ini_File}" W "W is for"
	/ini "${Ini_File}" X "X is for"
	/ini "${Ini_File}" Y "Y is for"
	/ini "${Ini_File}" Z "Z is for"
/if (${Debug}) /echo |- Build_Alphabetized_Ini ==>
/RETURN

|----------------------------------------------------------------------------|
|- Build an ini file, of a given name, that contains alphabetized sections.	-|
|----------------------------------------------------------------------------|
|- @param Ini_File: Name of the ini file you want to create.				-|
|----------------------------------------------------------------------------|
SUB Build_Class_Ini(Ini_File)
/if (${Debug}) /echo |- Build_Class_Ini ==>
	/ini "${Ini_File}" Bard "Bard is for"
	/ini "${Ini_File}" Beastlord "Beastlord is for"
	/ini "${Ini_File}" Berserker "Berserker is for"
	/ini "${Ini_File}" Cleric "Cleric is for"
	/ini "${Ini_File}" Druid "Druid is for"
	/ini "${Ini_File}" Enchanter "Enchanter is for"
	/ini "${Ini_File}" Magician "Magician is for"
	/ini "${Ini_File}" Monk "Monk is for"
	/ini "${Ini_File}" Necromancer "Necromancer is for"
	/ini "${Ini_File}" Paladin "Paladin is for"
	/ini "${Ini_File}" Ranger "Ranger is for"
	/ini "${Ini_File}" Rogue "Rogue is for"
	/ini "${Ini_File}" Shadowknight "Shadowknight is for"
	/ini "${Ini_File}" Shaman "Shaman is for"
	/ini "${Ini_File}" Warrior "Warrior is for"
	/ini "${Ini_File}" Wizard "Wizard is for"
/if (${Debug}) /echo |- Build_Class_Ini ==>
/RETURN

#event sendInstanceInvites "#1# tells you, 'dzadd'"
SUB event_sendInstanceInvites(line, ChatSender)
	/dzadd ${ChatSender}
/return
