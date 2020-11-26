|----------------------------------------------------------------------------|
| Cleric Functions
|----------------------------------------------------------------------------|
#event manastone "#*# tells you, 'mstone#*#'"
SUB event_manastone
/if (${Me.Sitting}) {
  		/sit off
		}
	/casting "Holy Elixir" "-targetid|${Me.ID}"
	:Loop
	/if (${Me.PctMana} < 100) {
			/casting "Manastone"
			/goto :Loop
	}
/return
|----------------------------------------------------------------------------|
SUB check_Yaulp
/if (${Debug}) /echo |- check_clrYaulp ==>
  /if (${AutoYaulp} && !${medBreak})  {
    /declare castName string local ${yaulpSpell.Arg[1,/]}
    /if (!${Bool[${Me.Buff[${castName}]}]} && ${Me.PctMana} < 95 && ${Spell[${castName}].NewStacks}) {
      /if (${Target.ID}) /declare tempTarget int local ${Target.ID}
      /casting "${castName}" -invis
    }
  }
/if (${Debug}) /echo <== check_clrYaulp -|
/RETURN
|----------------------------------------------------------------------------|
sub check_DivineArb
/if (${Debug}) /echo |- check_DivineArb ==>
  /if (${Defined[dArbArray2D]}) {
    /if (${DivineArb_Pct} && ${Group}>1 && ${dArbArray2D.Size[1]}>0) {
      /declare i int local
      /declare s int local
      /for i 0 to ${Group}
        /if (${SpawnCount[pc ${Group.Member[${i}]}]}) {
          /if (${SpawnCount[ID ${Spawn[${Group.Member[${i}]}].ID} radius 70]} > 0 && ${NetBots[${Group.Member[${i}]}].Zone} == ${Zone.ID} && (${NetBots[${Group.Member[${i}]}].PctHPs} < ${DivineArb_Pct} || ${Spawn[pc ${Group.Member[${i}]}].PctHPs} < ${DivineArb_Pct})) {
        /for s 1 to ${dArbArray2D.Size[1]}
        /call check_Ready "dArbArray2D" ${s}
        /if (${c_Ready}) {
          /if (${Me.Casting.ID}) /call interrupt
          /bc [+g+] Casting ${dArbArray2D[${s},${iCastName}]} for ${Group.Member[${i}]}-NB:${NetBots[${Group.Member[${i}]}].PctHPs},SP:${Spawn[pc ${Group.Member[${i}]}].PctHPs}
          /call e3_Cast ${Me.ID} "dArbArray2D" ${s}
              }
        /if (!${ActionTaken}) /next s
          }
        }
      /if (!${ActionTaken}) /next i
    }
  }
/if (${Debug}) /echo <== check_DivineArb -|
/return
|----------------------------------------------------------------------------|
sub check_celestialRegen
/if (${Debug}) /echo |- check_celestialRegen ==>
    /if (${celestialRegen_Pct} && ${Group}>2 && ${Me.AltAbilityReady[Celestial Regeneration]}) {
    /declare groupSize int local
    /declare averageHP int local
    /declare i int local
    | Count group members in zone
    /for i 0 to ${Group}
      /if (${SpawnCount[pc ${Group.Member[${i}]}]}) {
        /varcalc averageHP ${averageHP} + ${If[${NetBots[${Group.Member[${i}]}].ID},${NetBots[${Group.Member[${i}]}].PctHPs},${Group.Member[${i}].PctHPs}]}
        /varcalc groupSize ${groupSize} + 1
      }
    /next i
    
    /if (${groupSize}) {
      /varcalc averageHP ${averageHP} / ${groupSize}
      /if (${averageHP} < ${celestialRegen_Pct})  /casting "Celestial Regeneration|alt" "-targetid|${Me.ID}" -maxtries|2
    }
  }
/if (${Debug}) /echo <== check_celestialRegen -|
/return
|----------------------------------------------------------------------------|
#event wordHeal "<#1#> WordHeal"
sub Event_wordHeal(line, ChatSender)
  /if (!${Me.Class.ShortName.Equal[CLR]}) /return
  /if (!${checkEventArgs[${ChatSender},${line},UZR,""]}) /return
  /delay 5s !${Me.Casting.ID}
  /call check_Ready "wordHeal2D" 1
  /if (${c_Ready}) {
    /if (${check_Mana["wordHeal2D",1]}) {
      /bc [+g+] Casting ${wordHeal2D[1,${iCastName}]}
      /call e3_Cast ${Me.ID} "wordHeal2D" 1
    }
  }
/return
|----------------------------------------------------------------------------|
#event Rez "<#1#> Rez"
#event Rez "<#*#> Rez On #1#"
#event Rez "#1# tells you, 'Rez'"
#event Rez "#*# tells you, 'Rez On #1#'"
#event Rez "#1# tells the group, 'Rez'"
sub Event_Rez(line, ChatSender)
  /if (${ChatSender.Left[1].Compare[ ]}<0) /varset ChatSender ${ChatSender.Right[-2].Left[-1]}
  /if (${Me.Class.ShortName.Equal[CLR]} && ${Spawn[${ChatSender}'s].ID}) {
    /varset canRez 0
    /call defineRezSpell
    /call TrueTarget ${Spawn[${ChatSender}'s].ID}
    |/echo ${Target.ID}
    /call check_CorpseRezzable
    /if (${canRez}==1) {
      :checkReady
      /call check_Ready "rezArray2D" 1
      /if ((!${c_Ready} || !${check_Mana["rezArray2D",1]}) && ${Me.CombatState.NotEqual[COMBAT]}) {
        /if (!${Me.Sitting}) /sit on
        /delay 1s
        /goto :checkReady
      }
      /tell ${ChatSender} Wait4Rez
      /call e3_cast ${Spawn[${ChatSender}'s].ID} "rezArray2D" 1
    } else {
      /tell ${ChatSender} Your corpse ${Target.Name} is not eligible for rez
    }
  } else /if (!${Spawn[${ChatSender}'s].ID}) {
    /tell ${ChatSender} I don't see any of your corpses in ${Zone}.
  }
/return
|----------------------------------------------------------------------------|
#EVENT AE_Rez "[MQ2] AE Rez"
#EVENT AE_Rez "#1# tells you, 'AERez'"
SUB EVENT_AE_Rez(line, ChatSender)
/if (${Debug} || ${Debug_Basics}) /echo |- EVENT_AE_Rez ==>
  /call defineRezSpell
  /call BuildCorpseList
  /if (!${CorpseListSize}) {
    /echo I couldn't find any corpses to resurrect...
  } else {
    /if (${NetAdvPath.Status}==1) /squelch /netfollow off
    /if (${Stick.Active}) /squelch /stick off

    /declare numIterations int local 2
    /declare i int local
    /declare n int local
    /declare playerName string local
    /declare corpseID int local

    /for n 1 to ${numIterations}
      /for i 1 to ${CorpseListSize}
        /varset canRez 0
        /varset corpseID ${CorpseListPriority[${i}]}
        /varset playerName ${Spawn[id ${corpseID}].CleanName}
        /varset playerName ${playerName.Arg[1,']}

        /if (${corpseList.Find[${corpseID},]}) {
          /echo I have already rezzed ${playerName}-${corpseID}
          /goto :skipRez
        }
        /if (${otherRezList.Find[${corpseID},]}) {
          /echo Someone is already rezzing ${playerName}-${corpseID}
          /goto :skipRez
        }
        /call TrueTarget ${corpseID}
        /delay 1
        /call check_CorpseRezzable

        /if (${canRez}!=1) {
          /echo ${Target.Name} is not eligible for rez
          /goto :skipRez
        } else {
          /tell ${playerName} Wait4Rez
          /delay 3
          /if (${Target.Distance} > 15 && ${Target.Distance} < 100) {
            /corpse
            /delay 2
            /if (${Target.Distance} > 35) {
              /tell ${playerName} skipping rez, corpse drag failed: ${Target.Distance} distance
              /goto :skipRez
            }
          }
          :checkReady
          /call check_Ready "rezArray2D" 1
          /if ((!${c_Ready} || !${check_Mana["rezArray2D",1]}) && ${Me.CombatState.NotEqual[COMBAT]}) {
            /if (!${Me.Sitting}) /sit on
            /delay 1s
            /goto :checkReady
          }
          |/gu oRezList ${otherRezList}
          /if (${corpseList.Find[${corpseID},]}) {
            /echo I have already rezzed ${playerName}-${corpseID}
            /goto :skipRez
          }
          /if (${otherRezList.Find[${corpseID},]}) {
            /echo Someone is already rezzing ${playerName}-${corpseID}
            /goto :skipRez
          }
          /docommand ${ChatToggle}  Resurrecting-${corpseID} ${playerName}
          /squelch /bca //RezzingCorpseID ${Me} ${corpseID}
          /call e3_cast ${corpseID} "rezArray2D" 1
          /if (${castReturn.Equal[CAST_SUCCESS]}) /varset corpseList ${corpseList}${corpseID},
        }
      :skipRez
    /next i
    /call BuildCorpseList
    /varset otherRezList
    /varset corpseList
  /next n
  /docommand ${ChatToggle} All nearby corpses have been resurrected.
 }
/if (${Debug} || ${Debug_Basics}) /echo <== EVENT_AE_Rez -|
/RETURN
|----------------------------------------------------------------------------|
#bind otherRezzing /RezzingCorpseID
SUB Bind_otherRezzing(whoIsRezzing,corpse_id)
  /if (${Me.Class.ShortName.Equal[CLR]}) {
    /echo ${whoIsRezzing} rezzing ${corpse_id}, added to otherRezList
    /varset otherRezList ${otherRezList}${corpse_id},
  }
/return
|----------------------------------------------------------------------------|
#event CorpseExpired "This corpse #1# be resurrected."
Sub Event_CorpseExpired(line, corpseExpired)
/if (${Debug}) /echo |- EVENT_CorpseExpired
  /if (${corpseExpired.Equal[cannot]}) {
    /varset canRez 2
  } else  {
    /varset canRez 1
  }
/return
|----------------------------------------------------------------------------|
Sub check_CorpseRezzable
  /declare conTimer timer local 5s
  /doevents CorpseExpired flush
  /varset canRez 0
  :waitForExpired
  /consider
  /delay 2
  /doevents CorpseExpired
  /if (${canRez}==0 && ${conTimer}) {
    /goto :waitForExpired
  }
/RETURN
|----------------------------------------------------------------------------|
Sub BuildCorpseList
  /declare c int local
  /declare p int local 1
  /declare cID int local
  /declare CorpseList[100] int local 0
  /if (${Defined[CorpseListPriority]}) /deletevar CorpseListPriority
  /declare CorpseListPriority[100] int outer 0
  /varset CorpseListSize 0
  /if (!${SpawnCount[pccorpse radius 100]}) /return
  /for c 1 to ${SpawnCount[pccorpse radius 100]}
    /varset cID ${NearestSpawn[${c},pccorpse radius 100].ID}
      /varcalc CorpseListSize ${CorpseListSize}+1
      /varset CorpseList[${CorpseListSize}] ${cID}
  /next c
  |add clr
  /for c 1 to ${CorpseListSize}
    /if (${Spawn[id ${CorpseList[${c}]}].Class.ShortName.Equal[CLR]}) {
      /varset CorpseListPriority[${p}] ${CorpseList[${c}]}
      /varcalc p ${p}+1
    }
  /next c
  |add war/dru/shm
  /for c 1 to ${CorpseListSize}
  /if (${Select[${Spawn[id ${CorpseList[${c}]}].Class.ShortName},DRU,SHM,WAR]}) {
    /varset CorpseListPriority[${p}] ${CorpseList[${c}]}
    /varcalc p ${p}+1
  }
  /next c
  |add bst,enc,pal,rng,shd
  /for c 1 to ${CorpseListSize}
  /if (${Select[${Spawn[id ${CorpseList[${c}]}].Class.ShortName},PAL,SHD,RNG,BST,ENC,MAG,WIZ,NEC]}) {
    /varset CorpseListPriority[${p}] ${CorpseList[${c}]}
    /varcalc p ${p}+1
  }
  /next c
  |add the rest
  /for c 1 to ${CorpseListSize}
  /if (${Select[${Spawn[id ${CorpseList[${c}]}].Class.ShortName},ROG,MNK,BER,BRD]}) {
    /varset CorpseListPriority[${p}] ${CorpseList[${c}]}
    /varcalc p ${p}+1
  }
  /next c
|  /for c 1 to ${CorpseListSize}
|    /echo c ${c}:  cl-${CorpseList[${c}]}-${Spawn[id ${CorpseList[${c}]}].Name} clp ${CorpseListPriority[${c}]}-${Spawn[id ${CorpseListPriority[${c}]}].Name}
|  /next c
/return
|----------------------------------------------------------------------------|
Sub defineRezSpell
  /declare rezSpell string local
  /declare i int local
  /if (${FindItem[Water Sprinkler of Nem Ankh].ID}) {
    /varset rezSpell Water Sprinkler of Nem Ankh
  } else /if (${Me.AltAbility[Blessing of Resurrection].ID}) {
    /varset rezSpell Blessing of Resurrection
  } else /if (${Me.Book[Spiritual Awakening]}) {
    /varset rezSpell Spiritual Awakening
  } else /if (${Me.Book[Reviviscence]}) {
    /varset rezSpell Reviviscence
  } else /if (${Me.Book[Resurrection]}) {
    /varset rezSpell Resurrection
  } else /if (${Me.Book[Restoration]}) {
    /varset rezSpell Restoration
  } else /if (${Me.Book[Resuscitate]}) {
    /varset rezSpell Resuscitate
  } else /if (${Me.Book[Renewal]}) {
    /varset rezSpell Renewal
  } else /if (${Me.Book[Revive]}) {
    /varset rezSpell Revive
  } else /if (${Me.Book[Reparation]}) {
    /varset rezSpell Reparation
  } else /if (${Me.Book[Reconstitution]}) {
    /varset rezSpell Reconstitution
  } else /if (${Me.Book[Reanimation]}) {
    /varset rezSpell Reanimation
  }
  /if (${Bool[${rezSpell}]}) {
    /if (${Defined[rezArray]}) /deletevar rezArray
    /declare rezArray[1] string outer ${rezSpell}
    /if (${rezArray.Size}) {
      /call BuildSpellArray "rezArray" "rezArray2D"
      /for i 1 to ${rezArray2D.Size[1]}
        /varset rezArray2D[${i},${iCastInvis}] 1
      /next i
      }
  } else {
    /bc I do not have a resurrection spell available
  }
/RETURN
|----------------------------------------------------------------------------|
Sub CLR_Setup
/if (${Debug}) /echo |- CLR_Setup ==>
  /declare canRez int outer 0
  /declare CorpseListSize int outer 0
  /declare otherRezList string outer
  /declare corpseList string outer
  /call iniToVarV "${Character_Ini},Cleric,Divine Arbitration (Pct)" DivineArb_Pct int outer
  /call iniToVarV "${Character_Ini},Cleric,Celestial Regeneration (Pct)" celestialRegen_Pct int outer
  /call iniToVarV "${Character_Ini},Cleric,Auto-Yaulp (On/Off)" AutoYaulp bool outer
  /call iniToVarV "${Character_Ini},Cleric,Yaulp Spell" yaulpSpell string outer
  /if (${AutoYaulp}) {
    /if (!${Defined[yaulpSpell]} || !${yaulpSpell.Length} || !${Me.Book[${yaulpSpell.Arg[1,/]}]}) {
      /echo ERROR! ${Character_Ini} > [Cleric] > Yaulp Spell is not correctly defined.  Please check this entry, and restart the macro.
      /varset AutoYaulp FALSE
    }
  }
  /if (${Bool[${Me.Book[Word Of Vivification]}]}) {
    /call BuildArray wordHeal "Word Of Vivification"
    /call BuildSpellArray "wordHeal" "wordHeal2D"
  }
  /if (${Bool[${FindItem[Aegis of Superior Divinity].InvSlot}]}) /call BuildArray dArbArray "Aegis of Superior Divinity"
  /if (${Bool[${FindItem[Harmony of the Soul].InvSlot}]}) /call BuildArray dArbArray "Harmony of the Soul"
  /if (${Bool[${Me.AltAbility[Divine Arbitration]}]}) /call BuildArray dArbArray "Divine Arbitration"
  /if (${Defined[dArbArray]}) /call BuildSpellArray "dArbArray" "dArbArray2D"
  /declare coh_dist int outer 150
        /call iniToVarV "${Character_Ini},Cleric,Auto-Pet Weapons (On/Off)" autoWeapons bool outer
        /call IniToArrayV "${Character_Ini},Cleric,Summoned Pet Hammer#" petItems
        /declare mageRangedSwap bool outer FALSE
        /if (${Defined[petItems]}) /call BuildSpellArray "petItems"     "petItems2D"
/if (${Debug}) /echo <== CLR_Setup -|
/return
|----------------------------------------------------------------------------|
SUB CLR_MacroSettings
/if (${Debug}) /echo |- CLR_MacroSettings ==>
  /call WriteToIni "${advSettings_Ini},CLR Functions,CLR Function#1" "check_DivineArb" 1
  /call WriteToIni "${advSettings_Ini},CLR Functions,CLR Function#2" "check_healChain" 1
  /call WriteToIni "${advSettings_Ini},CLR Functions,CLR Function#3" "check_Heals" 1
  /call WriteToIni "${advSettings_Ini},CLR Functions,CLR Function#4" "check_Cures" 1
  /call WriteToIni "${advSettings_Ini},CLR Functions,CLR Function#5" "check_celestialRegen" 1
  /call WriteToIni "${advSettings_Ini},CLR Functions,CLR Function#6" "check_Buffs" 1
  /call WriteToIni "${advSettings_Ini},CLR Functions,CLR Function#7" "check_Burns" 1
  /call WriteToIni "${advSettings_Ini},CLR Functions,CLR Function#8" "check_Yaulp" 1
  /call WriteToIni "${advSettings_Ini},CLR Functions,CLR Function#9" "check_Debuffs" 1
  /call WriteToIni "${advSettings_Ini},CLR Functions,CLR Function#10" "check_Nukes" 1
  /call WriteToIni "${advSettings_Ini},CLR Functions,CLR Function#11" "check_Pets" 1
  /call WriteToIni "${advSettings_Ini},CLR Functions,CLR Function#12" "check_SummonItems" 1
  /call WriteToIni "${advSettings_Ini},CLR Functions,CLR Function#13" "check_Food" 1
/if (${Debug}) /echo <== CLR_MacroSettings -|
/RETURN
|----------------------------------------------------------------------------|
SUB CLR_CharacterSettings
/if (${Debug}) /echo |- CLR_CharacterSettings ==>
  /call WriteToIni "${Character_Ini},Cleric,Divine Arbitration (Pct)" 35 0
  /call WriteToIni "${Character_Ini},Cleric,Celestial Regeneration (Pct)" 0 0
  /call WriteToIni "${Character_Ini},Cleric,Auto-Yaulp (On/Off)" Off 0
  /call WriteToIni "${Character_Ini},Cleric,Yaulp Spell"
  /call WriteToIni "${Character_Ini},Cleric,Auto-Pet Weapons (On/Off)"
  /call WriteToIni "${Character_Ini},Cleric,Summoned Pet Hammer"
/if (${Debug}) /echo <== CLR_CharacterSettings -|
/RETURN
|----------------------------------------------------------------------------|
Sub CLR_Aliases
/if (${Debug}) /echo |- CLR_Aliases ==>
  /noparse /squelch /alias /RezIt /bc Rez on ${Target.CleanName.Arg[1,']}
  /squelch /alias /AERez /echo AE Rez
  /squelch /alias /dpw /bc Distribute Pet Weapons
  /squelch /alias /PetWeapons /bc Distribute Pet Weapons
/if (${Debug}) /echo <== CLR_Aliases -|
/return
|----------------------------------------------------------------------------|
#event AE_CR "#*# tells you, 'AECR'"
sub Event_AE_CR(string line)
/if (${Me.Class.ShortName.Equal[CLR]}) {
   /if (${Me.AltAbilityReady[Mass Group Buff]} && ${Me.AltAbilityReady[Celestial Regeneration]}) {
     /bc MGB Celestial Regeneration inc...
     /casting "Mass Group Buff|alt" -maxtries|3
     /delay 5 !${Me.AltAbilityReady[Mass Group Buff]}
     /delay 5
     /casting "Celestial Regeneration|alt" -maxtries|3
     /delay 5 !${Me.AltAbilityReady[Celestial Regeneration]}
     /casting "Celestial Regeneration|alt" -maxtries|3
     /rs MGB Celestial Regeneration inc...
   } else /if (!${Me.AltAbilityReady[Mass Group Buff]}) {
      /bc Mass Group Buff is not available...
   } else /bc Celestial Regeneration is not available...
}
/return
|----------------------------------------------------------------------------|
#event AE_HP "#*# tells you, 'AEHP'"
sub Event_AE_HP(string line)
/if (${Me.Class.ShortName.Equal[CLR]}) {
   /if (${Me.AltAbilityReady[Mass Group Buff]} && ${Bool[${Me.Book[Ancient: Gift of Aegolism]}]}) {
     /bc MGB Ancient: Gift of Aegolism inc...
     /casting "Mass Group Buff|alt" "-maxtries|3"
     /delay 5 !${Me.AltAbilityReady[Mass Group Buff]}
     /delay 5
     /casting "Ancient: Gift of Aegolism" "-maxtries|3"
     /delay 5 !${Me.SpellReady[Ancient: Gift of Aegolism]}
     /casting "Ancient: Gift of Aegolism" "-maxtries|3"
   } else /if (!${Me.AltAbilityReady[Mass Group Buff]}) {
      /bc Mass Group Buff is not available...
   } else /bc Ancient: Gift of Aegolism is not in your Spell Book noob...
}
/return
|----------------------------------------------------------------------------|
| Cleric Hammer Functions
|----------------------------------------------------------------------------|
#event petHammer "#1# tells you, 'Pet Hammer#*#'"
#event petHammer "#1# tells you, 'Distribute Pet Hammer#*#'"
#event petHammer "<#1#> Pet Hammer#*#"
#event petHammer "<#1#> Distribute Pet Hammer#*#"
sub event_petHammer(line, ChatSender)
/if (${Debug} || ${Debug_CLR}) /echo |- event_petHammer ==>
  /declare wasFollowing bool local ${Following}
  /varset Following FALSE
  |/varset Debug_CLR TRUE
  /declare singlePet bool local TRUE
  /if (${Defined[petIDArray]}) /deletevar petIDArray
  /if (${Bool[${line.Find[Distribute]}]}) /varset singlePet FALSE

  /if (!${checkEventArgs[${ChatSender},${line},ZR]}) /return
  /if (!${Defined[petItems2D]}) {
    /if (${Debug} || ${Debug_CLR}) /echo I do not have any Pet Items defined...
    /if (${singlePet} && ${c_eventArgChatSender.NotEqual[${Me}]}) /tell ${c_eventArgChatSender} I do not have any Pet Items defined...
  } else /if (!${SpawnCount[${c_eventArgChatSender}]}) {
    /echo ${c_eventArgChatSender} is not in the zone
    /if (${c_eventArgChatSender.NotEqual[${Me}]}) /tell ${c_eventArgChatSender} We are not in the same zone...
  } else /if (!${singlePet} && !${SpawnCount[pcpet radius 100]}) {
      /echo There are no PC Pets withing a radius of 100
      /if (${c_eventArgChatSender.NotEqual[${Me}]}) /tell ${c_eventArgChatSender} No pets a radius of 100...
  } else /if (${singlePet} && ${Spawn[${c_eventArgChatSender}].Pet.Distance}>100) {
      /if (${c_eventArgChatSender.NotEqual[${Me}]}) /tell ${c_eventArgChatSender} Your pet is not in a radius of 100...
  } else {
    /if (!${singlePet} && ${c_eventArgChatSender.NotEqual[${Me}]}) /tell ${c_eventArgChatSender} Handing out pet weapons...
    /declare originalLoc string local ${Me.Loc}
    /declare p int local
    |if distribute, give to everyone, else just give to the single chatsender
    /for p 1 to ${SpawnCount[pcpet radius 100]}
      /if (!${NearestSpawn[${p},pcpet].Name.Find[familiar]}) {
        |/echo ${p} ${NearestSpawn[${p},pcpet]} ${NearestSpawn[${p},pcpet].Name} ${NearestSpawn[${p},pcpet].Master} ${c_eventArgChatSender}
        |/echo los ${NearestSpawn[${p},pcpet].LineOfSight} d ${Bool[${line.Find[Distribute]}]}
        /if (${NearestSpawn[${p},pcpet].LineOfSight}) {
          /if (!${singlePet}) {
            /call BuildArray petIDArray ${NearestSpawn[${p},pcpet].ID}
            |/echo all added ${NearestSpawn[${p},pcpet].Name}-${NearestSpawn[${p},pcpet].Master} ${petIDArray.Size}
          } else /if (${NearestSpawn[${p},pcpet].Master.CleanName.Equal[${c_eventArgChatSender}]}) {
            /call BuildArray petIDArray ${NearestSpawn[${p},pcpet].ID}
            |/echo you added ${NearestSpawn[${p},pcpet].Name}-${NearestSpawn[${p},pcpet].Master} ${petIDArray.Size}
          }
        }
      }
    /next p
    /if (${Defined[petIDArray]}) {
      /if (${Stick.Active}) /squelch /stick off
      /if (${NetAdvPath.Status}==1) /squelch /netfollow off
      /call give_PetsHammer FALSE
    }

    /if (${Math.Distance[${originalLoc}]}>20) /call MoveToLoc ${originalLoc} 50 20
    /if (${c_eventArgChatSender.NotEqual[${Me}]}) /tell ${c_eventArgChatSender} Finished arming pets
    /varset Following ${wasFollowing}
  }
  |/varset Debug_CLR FALSE
/if (${Debug} || ${Debug_CLR}) /echo <== event_petHammer -|
/return
|----------------------------------------------------------------------------|
SUB give_PetsHammer(autoWeap)
  /declare p int local
  /declare w int local
  /declare allWeapSuccess bool local
  /declare retryWeapTimer timer local 14s

  /for p 1 to ${petIDArray.Size}
    |/echo ${p}-${petIDArray[${p}]}-${Spawn[id ${petIDArray[${p}]}].Name}-${Spawn[id ${petIDArray[${p}]}].Master}
    /if (${SpawnCount[id ${petIDArray[${p}]}]}) {
      /varset allWeapSuccess TRUE
      /if (${Spawn[id ${petIDArray[${p}]}].Master.CleanName.NotEqual[${Me}]}) /tell ${Spawn[id ${petIDArray[${p}]}].Master} Incoming pet Hammer, DO NOT MOVE OR YOU WILL BREAK ME I'LL TELL YOU WHEN I'M DONE!
      /call TrueTarget ${petIDArray[${p}]}
      /if (${Target.ID} && ${Target.Distance}>19) /call MoveToLoc ${Target.Y} ${Target.X} 50 15
        /for w 1 to ${petItems2D.Size[1]}
          :SummonWeap_Loop
          /if (${petItems2D[${w},${iCastName}].Equal["Blazing Stone of Demise"]} && ${FindItemCount[=Summoned: Burning Shank]}) {
            /call SwapItem "Summoned: Burning Shank" "Cursor"
          } else /if (${Bool[${FindItem[=${petItems2D[${w},${iCastName}]}]}]}) {
            /ItemNotify "${petItems2D[${w},${iCastName}]}" leftmouseup
          } else /if (${Bool[${FindItem[=Summoned: ${petItems2D[${w},${iCastName}]}]}]}) {
            /ItemNotify "Summoned: ${petItems2D[${w},${iCastName}]}" leftmouseup
          } else {
            /call check_Ready "petItems2D" ${w}
            /if (${c_Ready}) {
              /if (${check_Mana["petItems2D",${w}]}) {
                /call e3_Cast ${Me.ID} "petItems2D" ${w}
                /delay 5s ${Cursor.ID}
              }
            }
          }
          /if (!${Cursor.ID}) {
            /if (${retryWeapTimer}) {
              /goto :SummonWeap_Loop
            } else {
              /varset allWeapSuccess FALSE
            }
          } else /if (${Cursor.ID}) {
            /call TrueTarget ${petIDArray[${p}]}
            :OpenTrade_Loop
            /click left target
            /delay 1s ${Window[GiveWnd].Open}
            /if (!${Window[GiveWnd].Open}) {
              /if (${retryWeapTimer} && ${Cursor.ID}) {
                /goto :OpenTrade_Loop
              } else {
                /echo Failed to open trade with ${Target.CleanName}.
              }
            } else {
              :WaitAccept_Loop
              /notify GiveWnd GVW_Give_Button LeftMouseUp
              /delay 1s !${Window[GiveWnd].Open}
              /if (${Window[GiveWnd].Open}) {
                /if (${retryWeapTimer}) {
                  /goto :WaitAccept_Loop
                } else {
                  /echo Failed to open trade with ${Target.CleanName}.
                }
              }
            }
          }
        /next w
      /if (${autoWeap} && ${allWeapSuccess}) {
        /call WriteToIni "${MacroData_Ini},${Me.CleanName}-${MacroQuest.Server},Auto-Pet Weapons-${Spawn[id ${petIDArray[${p}]}].Master}" "${Spawn[id ${petIDArray[${p}]}].CleanName}" 1
      }
    }
  /next p
/return
|----------------------------------------------------------------------------|
SUB check_SummonItems
/if (${Debug} || ${Debug_CLR}) /echo |- check_SummonItems ==>
/if (!${Me.CombatState.Equal[COMBAT]}) {
  /if (${summonMoltenOrb}) {
    /if (!${Me.Moving} && !${combatTimer} && !${activeTimer} && !${rebuffTimer}) {
        /if (!${FindItemCount[=Molten Orb]}) {
          /call check_Ready "moltenOrb2D" 1
          /if (${c_Ready}) {
            /if (${check_Mana["moltenOrb2D",1]}) {
              /call e3_Cast ${Me.ID} "moltenOrb2D" 1
              /delay 2s ${Cursor.ID}
              /if (${Cursor.ID}) /call ClearCursor
            }
          }
        }
    }
  }

        /if (${autoWeapons} && ${Macro.RunTime} > 10) {
                /if (!${Me.Moving} && !${combatTimer} && !${activeTimer} && !${rebuffTimer}) {
                        /declare i int local
      /declare p int local
      /declare masterName string local
      /declare petName string local
      /declare petRace string local
      /declare petID int local
      /if (${Defined[petIDArray]}) /deletevar petIDArray
      /for p 1 to ${SpawnCount[pcpet radius 100]}
        /varset masterName ${NearestSpawn[${p},pcpet].Master}
        /varset petName ${NearestSpawn[${p},pcpet].CleanName}
        /varset petID ${NearestSpawn[${p},pcpet].ID}
        /varset petRace ${NearestSpawn[${p},pcpet].Race}
        /if (!${Bool[${petName}]} || !${Bool[${masterName}]} || ${petName.Find[familiar]} || ${petRace.Equal[Gnomework]}) /return
        /if (${NetBots[${masterName}].ID}) {
          /if (${Spawn[id ${petID}].LineOfSight}) {
            /if (${petName.Equal[${Ini[${MacroData_Ini},${Me.CleanName}-${MacroQuest.Server},Auto-Pet Weapons-${masterName}]}]}) {
                                                  /if (${Debug} || ${Debug_CLR}) /echo check_SummonItems: I have already given weapons to ${petName}
                                          } else {
              /call BuildArray petIDArray ${NearestSpawn[${p},pcpet].ID}
                                          }
                                  }
                    }
                  /next p
                  /if (${Defined[petIDArray]}) {
        /call give_PetsHammer TRUE
      }
          }
        }
}
/if (${Debug} || ${Debug_CLR}) /echo <== check_SummonItems -|
/RETURN
|----------------------------------------------------------------------------|
#event playerHammer "#1# tells you, 'player Hammer#*#'"
#event playerHammer "<#1#> Player Hammer#*#"
sub event_playerHammer(line, ChatSender)
/echo |- event_playerHammer ==>
      /target ${ChatSender}
	  /tell ${ChatSender} You need to be right next to me, the script doesn't make me come to you.
      /delay 3
	  /if (${Bool[${FindItem[=Hammer of Damnation]}]}) {
        /ItemNotify "Hammer of Damnation" leftmouseup
      } else {
		/cast "Hammer of Damnation"
	  }
      /delay 6s ${Cursor.ID}
      /click left target
      /delay 1s ${Window[GiveWnd].Open}
	  /notify GiveWnd GVW_Give_Button LeftMouseUp
	  /delay 4s
	  /if (${Cursor.ID}) /call ClearCursor
/return
|----------------------------------------------------------------------------|
Sub CLR_Background_Events
|**
        /if (${Bool[${Me.AltAbility[Companion of Necessity]}]} && ${Me.AltAbilityReady[Companion of Necessity]}) {
                /if (${Assisting} && ${Me.CombatState.Equal[Combat]} && ${Target.Named} && ${Target.Level} >= ${Math.Calc[${Me.Level}+3]} && !${Select[${Me.TargetOfTarget.Class},WAR,PAL,SHD]} && ${Spawn[${Target.ID}].PlayerState}==4) {
                        /if (${Me.Casting.ID}) /call interrupt
                        /delay 1s !${Me.Casting.ID}
                        /gu ${Target.Name} has aggro on ${Me.TargetOfTarget.Name}, casting Companion of Necessity
                        /casting "Companion of Necessity"
                }
        }
**|
  /doevents petHammer
  /doevents AE_CR
  /doevents AE_HP
  /doevents AE_Rez
  /doevents Rez
  /doevents manastone
  /doevents playerHammer
/return
