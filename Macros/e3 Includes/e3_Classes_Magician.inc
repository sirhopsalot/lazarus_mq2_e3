|----------------------------------------------------------------------------|
| Magician Functions
|----------------------------------------------------------------------------|
#event petWeapons "#1# tells you, 'Pet Weapon#*#'"
#event petWeapons "#1# tells you, 'Distribute Pet Weapon#*#'"
#event petWeapons "<#1#> Pet Weapon#*#"
#event petWeapons "<#1#> Distribute Pet Weapon#*#"
sub event_petWeapons(line, ChatSender)
/if (${Debug} || ${Debug_MAG}) /echo |- event_petWeapons ==>
  /declare wasFollowing bool local ${Following}
  /varset Following FALSE
  /varset Debug_MAG FALSE
  /declare singlePet bool local TRUE
  /if (${Defined[petIDArray]}) /deletevar petIDArray
  /if (${Bool[${line.Find[Distribute]}]}) /varset singlePet FALSE

  /if (!${checkEventArgs[${ChatSender},${line},ZR]}) /return
  /if (!${Defined[petItems2D]}) {
    /if (${Debug} || ${Debug_MAG}) /echo I do not have any Pet Items defined...
    /if (${singlePet} && ${c_eventArgChatSender.NotEqual[${Me}]}) /tell ${c_eventArgChatSender} I do not have any Pet Items defined...
  } else /if (!${SpawnCount[${c_eventArgChatSender}]}) {
    /echo ${c_eventArgChatSender} is not in the zone
    /if (${c_eventArgChatSender.NotEqual[${Me}]}) /tell ${c_eventArgChatSender} We are not in the same zone...
  } else /if (!${singlePet} && !${SpawnCount[pcpet radius 50]}) {
      /echo There are no PC Pets withing a radius of 50
      /if (${c_eventArgChatSender.NotEqual[${Me}]}) /tell ${c_eventArgChatSender} No pets a radius of 50...
  } else /if (${singlePet} && ${Spawn[${c_eventArgChatSender}].Pet.Distance}>100) {
      /if (${c_eventArgChatSender.NotEqual[${Me}]}) /tell ${c_eventArgChatSender} Your pet is not in a radius of 100...
  } else {
    /if (!${singlePet} && ${c_eventArgChatSender.NotEqual[${Me}]}) /tell ${c_eventArgChatSender} Handing out pet weapons...
    /declare originalLoc string local ${Me.Loc}
    /declare p int local
    |if distribute, give to everyone, else just give to the single chatsender
    /for p 1 to ${SpawnCount[pcpet radius 50]}
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
      /call give_PetsWeapons FALSE
    }

    /if (${Math.Distance[${originalLoc}]}>20) /call MoveToLoc ${originalLoc} 50 20
    /if (${c_eventArgChatSender.NotEqual[${Me}]}) {
		/delay 1s
		/tell ${c_eventArgChatSender} Finished arming pets. If anything is missing, ask for it directly. Ex: "/tell ${Me.CleanName} Grant Spectral Plate".
	}
    /varset Following ${wasFollowing}
  }
  |/varset Debug_MAG FALSE
/if (${Debug} || ${Debug_MAG}) /echo <== event_petWeapons -|
/return

#event armPets "#1# tells you, 'Arm Pets'"
#event armPets "#1# tells you, 'Arm Pets #2#'"
|#event armPets "<#1#> Arm Pets <#2#> <#3#>"
sub event_armPets(line, ChatSender, loadType)
 /echo |- event_armPets ==>
  
  /declare characterToWorkOn string local
  
  /if (${Bool[${loadType.Length}]}) {
   /varset characterToWorkOn ${loadType.Token[2, ]}
   /varset loadType ${loadType.Token[1, ]}
   /if (${characterToWorkOn.Equal[NULL]}) {
     |set to empty string
     /varset characterToWorkOn  
   }
   |/echo LoadType:${loadType}
   |/echo Character:${characterToWorkOn}
  }
  
  |first check that we have an empty slot
  /declare BackSlotThatIsOpen int 0
  /declare i int local
  /for i 1 to 10
  /if (!${Bool[${InvSlot[pack${i}].Item.ID}]}) {
    /varset BackSlotThatIsOpen ${i}
    /goto :bagslotcheckdone
  }
  /next i
  :bagslotcheckdone
  /if (${BackSlotThatIsOpen}==0) {
    |no empty bag slot available
    /tell ${ChatSender}  Arm Pets: No empty bag slot on mage. Exiting.
    /goto :armpetend
  } 
 

  /tell ${ChatSender} Starting the arming of group pets.
  |note for the weapons, you need na empty bag slot for this to work.
  /declare counter int local 0
  /declare bodytype string local
  /declare char_class string local
  /declare primaryWeapon string local
  /declare secondaryWeapon string local 

  |/echo Character:${characterToWorkOn}
  |/echo loadType:${loadType}

  /if (${Cursor.ID}) {
    /echo Arm Pets: Something is on your cursor, exiting.
    /tell ${ChatSender}  Arm Pets: Error, check mage for detail.
    /goto :armpetend
  }
  |/echo got to for loop: ${characterToWorkOn.Length} : ${characterToWorkOn.Length}>0]}
  /for counter 0 to 5
    |if character name is supplied, skip everything but the character to work on. 
    |/echo checking character: [${characterToWorkOn}] vs [${Spawn[${Group.Member[${counter}]}].Name}]
   
    /if (${characterToWorkOn.Length}>0 && !${characterToWorkOn.Equal[${Spawn[${Group.Member[${counter}]}].Name}]}) /continue
    
    /varset bodytype ${Spawn[${Group.Member[${counter}]}].Pet.Body}
    
    /if (${Spawn[${Group.Member[${counter}]}].Pet.ID} && !${Spawn[${Group.Member[${counter}]}].Pet.Name.Find[familiar]} && ${Spawn[${Group.Member[${counter}]}].Pet.Level}>1) {
      
      /bc Arm Pets: Working on ${Spawn[${Group.Member[${counter}]}].Name} 's pet
      /if (${Me.Book[Grant Enibik's Heirlooms]}) {
           /bc Arm Pets: Creating Heirlooms
           /call summonUnpackAndGive ${Spawn[${Group.Member[${counter}]}].Pet.ID} "Grant Enibik's Heirlooms" "Folded Pack of Enibik's Heirlooms"
      } 
      /if (${Cursor.ID}) {
        /bc Arm Pets: Something went wrong and something is on your cursor, exiting.
        /goto :armpetend
      }
      /if (${Me.Book[Grant Spectral Plate]}) {
        /bc Arm Pets: Creating Spectral Plate
        /call summonUnpackAndGive ${Spawn[${Group.Member[${counter}]}].Pet.ID} "Grant Spectral Plate" "Folded Pack of Spectral Plate"
      }
      /if (${Cursor.ID}) {
        /bc Arm Pets: Something went wrong and something is on your cursor, exiting.
        /goto :armpetend
      }
      
      /if (${Me.Book[Grant Spectral Armaments]}) {
        |/echo the line to parse is:${loadType}
        /varset char_class ${Spawn[${Group.Member[${counter}]}].Class} 
        /if (${char_class.Find[Shadow Knight]}) {
          
          |/echo Arm Pet: Doing ${char_class} loadout
          
          /if (${loadType.Find[fire]}) {

            /varset primaryWeapon "Summoned: Fist of Flame"
            /varset secondaryWeapon "Summoned: Spear of Maliciousness"

          } else /if (${loadType.Find[ice]}) {
            
            /varset primaryWeapon "Summoned: Orb of Chilling Water"
            /varset secondaryWeapon "Summoned: Spear of Maliciousness"
            
          } else /if (${loadType.Find[2dispel]}) {
            
            /varset primaryWeapon "Summoned: Wand of Dismissal"
            /varset secondaryWeapon "Summoned: Wand of Dismissal"

          } else {
            
            /varset primaryWeapon "Summoned: Wand of Dismissal"
            /varset secondaryWeapon "Summoned: Spear of Maliciousness"
        
          }
        
        } else /if (${char_class.Find[Druid]}) {
          |/echo Arm Pet: Doing ${char_class} loadout
          /if (${loadType.Find[fire]}) {

            /varset primaryWeapon "Summoned: Fist of Flame"
            /varset secondaryWeapon "Summoned: Spear of Maliciousness"

          } else /if (${loadType.Find[ice]}) {
            
            /varset primaryWeapon "Summoned: Orb of Chilling Water"
            /varset secondaryWeapon "Summoned: Spear of Maliciousness"
            
          } else /if (${loadType.Find[2dispel]}) {
            
            /varset primaryWeapon "Summoned: Wand of Dismissal"
            /varset secondaryWeapon "Summoned: Wand of Dismissal"

          } else {
            
            /varset primaryWeapon "Summoned: Wand of Dismissal"
            /varset secondaryWeapon "Summoned: Spear of Maliciousness"
        
          }
          |grouped mage/nec/enc here, will probably have to split out depending on new payloads
        } else /if (${char_class.Find[Magician]} || ${char_class.Find[Necro]} || ${char_class.Find[Enchanter]}) {
          |/echo Arm Pet: Doing ${char_class} loadout
          /if (${loadType.Find[fire]}) {

            /varset primaryWeapon "Summoned: Fist of Flame"
            /varset secondaryWeapon "Summoned: Fist of Flame"

          } else /if (${loadType.Find[ice]}) {
            
            /varset primaryWeapon "Summoned: Orb of Chilling Water"
            /varset secondaryWeapon "Summoned: Orb of Chilling Water"
            
          } else /if (${loadType.Find[2dispel]}) {
            
            /varset primaryWeapon "Summoned: Wand of Dismissal"
            /varset secondaryWeapon "Summoned: Wand of Dismissal"

          } else {
            
            /varset primaryWeapon "Summoned: Mace of Temporal Distortion"
            /varset secondaryWeapon "Summoned: Wand of Dismissal"
        
          }
          
        } else {
          |/echo Arm Pet: Doing Default ${char_class} loadout
          /varset primaryWeapon "Summoned: Wand of Dismissal"
          /varset secondaryWeapon "Summoned: Spear of Maliciousness"

        }

        /bc Arm Pets: Selected Weapons are ${primaryWeapon} and ${secondaryWeapon}
        /bc Arm Pets: Creating Weapons
        /call summonUnpackAndSelect ${Spawn[${Group.Member[${counter}]}].Pet.ID} "Grant Spectral Armaments" "Folded Pack of Spectral Armaments" ${primaryWeapon} ${secondaryWeapon}
      }
      /if (${Cursor.ID}) {
        /bc Arm Pets: Something went wrong and something is on your cursor, exiting.
        /goto :armpetend
      }

    }
    |/echo ${Spawn[${Group.Member[${counter}]}].Name} the ${Spawn[${Group.Member[${counter}]}].Class} has pet: ${Spawn[${Group.Member[${counter}]}].Pet.ID} which is ${bodytype}
  /next counter
  /tell ${ChatSender} Finished arming pets.
:armpetend
/if (${Debug} || ${Debug_MAG}) /echo <== event_armPets -|
/return
sub summonUnpackAndSelect(targetID, spellName, ItemName, primaryItem, secondaryItem)

      /if (${Defined[armPetArray]}) /deletevar armPetArray
      /declare armPetArray[1] string outer ${spellName}
      /if (${armPetArray.Size}) {
      
        /call BuildSpellArray "armPetArray" "armPetArray2D"
        /call e3_Cast ${Me.ID} "armPetArray2D" 1 False
        /while (!${Select[${castReturn},CAST_SUCCESS]}) {
             /call e3_Cast ${Me.ID} "armPetArray2D" 1 False
        }
        /if (${Defined[armPetArray]}) /deletevar armPetArray

        /delay 1s
        |/echo done casting, put in inventory
        /call ClearCursor
        /delay 1s
        |/echo unfolding the bag to cursor
        /ItemNotify "${ItemName}" rightmouseup
        /delay 4s 
        /call ClearCursor
        /delay 1s
        /itemnotify "${primaryItem}" leftmouseup
        /delay 5s ${Bool[${Cursor.ID}]}
        /if (!${Cursor.ID}) {
          /echo summonUnpackAndSelect: Could not load parimary item to cursor, exiting
          /return
        }

        |/echo getting the target of the pet
        /call TrueTarget ${targetID}
        /if (${Target.ID} && ${Target.Distance}>19) /call MoveToLoc ${Target.Y} ${Target.X} 50 15


        |/echo giving primary item to pet
        /call giveItemOnCursorToTarget
        /if (${Cursor.ID}) {
          /echo summonUnpackAndSelect: Something went wrong and something is on your cursor, exiting.
          /return
        }

        |check to see if we have a 2ndary item
        /if (!${Bool[${FindItem[${secondaryItem}]}]}) {
         
         |we don't have the 2ndary item in this pack, summon the pack again and try again
         |now delete the object
          /itemnotify "Pouch of Quellious" leftmouseup
          /delay 5s ${Bool[${Cursor.ID}]}
          /if (${Cursor.Name.Find[Pouch of Quellious]}) {
            /destroy
          } else {
            /echo summonUnpackAndSelect: Could not destroy bag, exiting
            /return
          }
           /if (${Defined[armPetArray]}) /deletevar armPetArray
          /declare armPetArray[1] string outer ${spellName}
          /call BuildSpellArray "armPetArray" "armPetArray2D"
          /call e3_Cast ${Me.ID} "armPetArray2D" 1 False
          /while (!${Select[${castReturn},CAST_SUCCESS]}) {
              /call e3_Cast ${Me.ID} "armPetArray2D" 1 False
          }
          /if (${Defined[armPetArray]}) /deletevar armPetArray
          /delay 1s
          |/echo done casting, put in inventory
          /call ClearCursor
          /delay 1s
          |/echo unfolding the bag to cursor
          /ItemNotify "${ItemName}" rightmouseup
          /delay 4s 
          /call ClearCursor
          /delay 1s
          /call TrueTarget ${targetID}
        } 
       
        |2ndary item
        /itemnotify "${secondaryItem}" leftmouseup
        /delay 5s ${Bool[${Cursor.ID}]}
        
        /if (!${Cursor.ID}) {
          /echo summonUnpackAndSelect: Could not load parimary item to cursor, exiting
          /return
        }

        
        |/echo giving secondary item to pet
        /call giveItemOnCursorToTarget
        /if (${Cursor.ID}) {
          /echo summonUnpackAndSelect: Something went wrong and something is on your cursor, exiting.
          /return
        }
      
        |now delete the object
        /itemnotify "Pouch of Quellious" leftmouseup
        /delay 5s ${Bool[${Cursor.ID}]}
        /if (${Cursor.Name.Find[Pouch of Quellious]}) {
           /destroy
        } else {
          /echo summonUnpackAndSelect: Could not destroy bag, exiting
          /return
        }

      }

/return
sub summonUnpackAndGive(targetID, spellName, ItemName)

     /if (${Defined[armPetArray]}) /deletevar armPetArray
      /declare armPetArray[1] string outer ${spellName}
      /if (${armPetArray.Size}) {
        |/echo building the spell array for heirlooms

        /call BuildSpellArray "armPetArray" "armPetArray2D"
        /call e3_Cast ${Me.ID} "armPetArray2D" 1 False
        /while (!${Select[${castReturn},CAST_SUCCESS]}) {
             /call e3_Cast ${Me.ID} "armPetArray2D" 1 False
        }
        
         /if (${Defined[armPetArray]}) /deletevar armPetArray
        /delay 1s
        |/echo done casting, put in inventory
        /autoinventory
        /delay 3s !${Bool[${Cursor.ID}]}
        /delay 1s
        |/echo unfolding the bag to cursor
        /ItemNotify "${ItemName}" rightmouseup
        /delay 3s
        |/echo getting the target of the pet
        /call TrueTarget ${targetID}
        /if (${Target.ID} && ${Target.Distance}>19) /call MoveToLoc ${Target.Y} ${Target.X} 50 15
        /call giveItemOnCursorToTarget
        /if (${Cursor.ID}) {
          /echo summonUnpackAndGive: Something went wrong and something is on your cursor, exiting.
          /return
        }
      }

/return
sub giveItemOnCursorToTarget()

    /if (${Defined[retryWeapTimer]}) /deletevar retryWeapTimer
    /declare retryWeapTimer timer local 10s

    |/echo trying to trade heirloom pack to pet
    :OpenTrade_Loop
    
    /click left target
    /delay 3s ${Window[GiveWnd].Open}
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

/return
|----------------------------------------------------------------------------|
SUB give_PetsWeapons(autoWeap)
  /declare p int local
  /declare w int local
  /declare allWeapSuccess bool local
  /declare retryWeapTimer timer local 10s
  /declare DebugPetWeapons bool local false

  /for p 1 to ${petIDArray.Size}
    |/echo ${p}-${petIDArray[${p}]}-${Spawn[id ${petIDArray[${p}]}].Name}-${Spawn[id ${petIDArray[${p}]}].Master}
    /if (${SpawnCount[id ${petIDArray[${p}]}]}) {
      /varset allWeapSuccess TRUE
      /if (${Spawn[id ${petIDArray[${p}]}].Master.CleanName.NotEqual[${Me}]}) /tell ${Spawn[id ${petIDArray[${p}]}].Master} Incoming pet weapons, hold still!
      /call TrueTarget ${petIDArray[${p}]}
      /if (${Target.ID} && ${Target.Distance}>19) /call MoveToLoc ${Target.Y} ${Target.X} 50 15
        /for w 1 to ${petItems2D.Size[1]}
          :SummonWeap_Loop
          /if (${petItems2D[${w},${iCastName}].Equal["Blazing Stone of Demise"]} && ${FindItemCount[=Summoned: Burning Shank]}) {
            /call SwapItem "Summoned: Burning Shank" "Cursor"
          } else /if (${Bool[${FindItem[=${petItems2D[${w},${iCastName}]}]}]} && ${Bool[${FindItem[=${petItems2D[${w},${iCastName}]}].NoRent}]}) {
            /ItemNotify "${petItems2D[${w},${iCastName}]}" leftmouseup
          } else /if (${Bool[${FindItem[=Summoned: ${petItems2D[${w},${iCastName}]}]}]} && ${Bool[${FindItem[=Summoned: ${petItems2D[${w},${iCastName}]}].NoRent}]}) {
            /ItemNotify "Summoned: ${petItems2D[${w},${iCastName}]}" leftmouseup
          } else {
            /call check_Ready "petItems2D" ${w}
            /if (${c_Ready}) {
              /if (${check_Mana["petItems2D",${w}]}) {
				/if (${DebugPetWeapons}) /echo Item = ${petItems2D[${w},${iCastName}]}
				/if (${Select[${petItems2D[${w},${iCastName}]},Grant Spectral Plate]}) {
					/if (${Bool[${FindItem[=Folded Pack of Spectral Plate]}]}) {
						/ItemNotify "Folded Pack of Spectral Plate" rightmouseup
					} else {
						/if (${DebugPetWeapons}) /echo Grant Spectral Plate
						/tar ${Me}
						/delay 5s ${Select[${Target},${Me}]}
						/call e3_Cast ${Me.ID} "petItems2D" ${w}
						/delay 5s ${Cursor.ID}
						/autoinventory
						/if (${Bool[${FindItem[=Folded Pack of Spectral Plate]}]}) {
							/ItemNotify "Folded Pack of Spectral Plate" rightmouseup
						}
					}
				} else {
					/call e3_Cast ${Me.ID} "petItems2D" ${w}
				}
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
			  /if (${c_eventArgChatSender.NotEqual[${Me}]}) {
				/tell ${Spawn[id ${petIDArray[${p}]}].Master} ---Giving "${petItems2D[${w},${iCastName}]}" to ${Spawn[id ${petIDArray[${p}]}].CleanName}.
			  }
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
/if (${Debug} || ${Debug_MAG}) /echo |- check_SummonItems ==>
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
						  /if (${Debug} || ${Debug_MAG}) /echo check_SummonItems: I have already given weapons to ${petName}
					  } else {
              /call BuildArray petIDArray ${NearestSpawn[${p},pcpet].ID}
					  }
				  }
		    }
		  /next p
		  /if (${Defined[petIDArray]}) {
        /call give_PetsWeapons TRUE
      }
	  }
	}
}
/if (${Debug} || ${Debug_MAG}) /echo <== check_SummonItems -|
/RETURN
|----------------------------------------------------------------------------|
#event auto_coh "#1# tells you, 'autocoh #2#'"
#event auto_coh "<#1#> autocoh #2#"
SUB event_auto_coh(line, ChatSender, toggleCoH)
  /declare i int local
  /if (${toggleCoH.Equal[on]}) {
    /if (${Bool[${Group.Member[${ChatSender}]}]}) {
      /if (${Defined[cohTargets]}) {
        /for i 1 to ${cohTargets.Size}
          /if (${cohTargets[${i}].Equal[${ChatSender}]}) {
            /docommand ${ChatToggle} ${ChatSender} is already in the AutoCoH array
            /return
          }
        /next i
      }
      /call BuildArray cohTargets "${ChatSender}"
      /docommand ${ChatToggle} Auto CoH Enabled on ${ChatSender}
    } else {
      /docommand ${ChatToggle} Auto CoH will not be enabled until you are in a group with this mage
    }
  } else {
    /if (${Defined[cohTargets]}) /call RemoveArrayElement cohTargets "${ChatSender}"
    /docommand ${ChatToggle} Auto CoH Disabled on ${ChatSender}
  }
/RETURN
|----------------------------------------------------------------------------|
SUB check_CoH
/if (${Debug} || ${Debug_MAG}) /echo |- MAG_checkCoH ==>
  /if (${Defined[cohTargets]}) {
	/if (${Debug_MAG}) /echo cohTargets=${cohTargets.Size}
    /declare t int local
    /declare s int local
    /for t 1 to ${cohTargets.Size}
      /if (${Bool[${Group.Member[${cohTargets[${t}]}]}]}) {
        /for s 1 to ${cohSpells2D.Size[1]}     
		  /if (${Debug_MAG}) /echo Checking ${cohTargets[${t}]}
		  /if (${Debug_MAG}) /echo Distance ${Spawn[pc =${cohTargets[${t}]}].Distance}
		  /if (${Debug_MAG}) /echo check_Mana ${check_Mana["cohSpells2D",${s}]}
            /if (${Spawn[pc =${cohTargets[${t}]}].Distance} >= ${coh_dist}) {
			  /call check_Ready "cohSpells2D" ${s}
			  /if (${c_Ready}) {
                /if (${check_Mana["cohSpells2D",${s}]}) {
                  /docommand ${ChatToggle} << Auto CoH casting on ${cohTargets[${t}]} >>
                  /g << Auto CoH casting on ${cohTargets[${t}]} >>
                  /call e3_cast ${Spawn[pc =${cohTargets[${t}]}].ID} "cohSpells2D" ${s}
                } else /if (${Debug} || ${Debug_MAG}) /docommand ${ChatToggle} << AutoCoH CoH is not ready for ${cohTargets[${t}]}  >>
              } else /if (${Debug} || ${Debug_MAG}) /docommand ${ChatToggle} << AutoCoH CoH is not ready for ${cohTargets[${t}]}  >>
			} else {
				/echo AutoCoh complete on ${cohTargets[${t}]}
				/call RemoveArrayElement cohTargets "${cohTargets[${t}]}"				
			}
        /if (!${ActionTaken})  /next s
      } else /if (${Debug} || ${Debug_MAG}) /docommand ${ChatToggle} << AutoCoH ${cohTargets[${t}]} is not in my group >>
    /if (!${ActionTaken})  /next t
  }
  /if (${Debug} || ${Debug_MAG}) /echo <== MAG_checkCoH -|
/RETURN
|----------------------------------------------------------------------------|
SUB MAG_Setup
/declare Debug_MAG bool outer FALSE
/if (${Debug} || ${Debug_MAG}) /echo |- MAG_Setup ==>
  /declare coh_dist int outer 150
	/call iniToVarV "${Character_Ini},Magician,Auto-Pet Weapons (On/Off)" autoWeapons bool outer
	/call IniToArrayV "${Character_Ini},Magician,Summoned Pet Item#" petItems
  /call iniToVarV "${Character_Ini},Magician,Auto-Summon Orb of Mastery (On/Off)" autoOrbOfMastery bool outer

	/declare mageRangedSwap bool outer FALSE
	/if (${Defined[petItems]}) /call BuildSpellArray "petItems"	"petItems2D"

  /if (${Bool[${Me.Book[Call of the Hero]}]}) {
    /declare cohSpells[1] string outer Call of the Hero
    /call BuildSpellArray "cohSpells" "cohSpells2D"
  }

  /if (${Bool[${Me.Book[Summon: Molten Orb]}]}) {
    /declare moltenOrb[1] string outer Summon: Molten Orb
    /call BuildSpellArray "moltenOrb" "moltenOrb2D"
  }

  | Orb of Mastery must be enabled and the toon must have the spell
  /if (${autoOrbOfMastery} && ${Bool[${Me.Book[Summon Orb]}]}) {
    /varset autoOrbOfMastery TRUE
  } else {
    /varset autoOrbOfMastery FALSE
  }

/if (${Debug} || ${Debug_MAG}) /echo <== MAG_Setup -|
/RETURN
|----------------------------------------------------------------------------|
SUB MAG_MacroSettings
/if (${Debug}) /echo |- MAG_MacroSettings ==>
  /call WriteToIni "${advSettings_Ini},MAG Functions,MAG Function#1" "check_CoH" 1
	/call WriteToIni "${advSettings_Ini},MAG Functions,MAG Function#2" "check_Buffs" 1
  /call WriteToIni "${advSettings_Ini},MAG Functions,MAG Function#3" "check_Burns" 1
	/call WriteToIni "${advSettings_Ini},MAG Functions,MAG Function#4" "check_Pets" 1
	/call WriteToIni "${advSettings_Ini},MAG Functions,MAG Function#5" "check_Debuffs" 1
	/call WriteToIni "${advSettings_Ini},MAG Functions,MAG Function#6" "check_Ae" 1
	/call WriteToIni "${advSettings_Ini},MAG Functions,MAG Function#7" "check_Nukes" 1
	/call WriteToIni "${advSettings_Ini},MAG Functions,MAG Function#8" "check_SummonItems" 1
  /call WriteToIni "${advSettings_Ini},MAG Functions,MAG Function#9" "check_OrbOfMasteryCharges" 1
  /call WriteToIni "${advSettings_Ini},MAG Functions,MAG Function#10" "check_Food" 1
/if (${Debug}) /echo <== MAG_MacroSettings -|
/RETURN
|----------------------------------------------------------------------------|
SUB MAG_CharacterSettings
/if (${Debug}) /echo |- MAG_CharacterSettings ==>
	/call WriteToIni "${Character_Ini},Magician,Auto-Pet Weapons (On/Off)"
  /call WriteToIni "${Character_Ini},Magician,Auto-Summon Orb of Mastery (On/Off)"
	/call WriteToIni "${Character_Ini},Magician,Summoned Pet Item"
/if (${Debug}) /echo <== MAG_CharacterSettings -|
/RETURN
|----------------------------------------------------------------------------|
SUB MAG_Aliases
/if (${Debug}) /echo |- MAG_Aliases ==>
	/squelch /alias /dpw /bc Distribute Pet Weapons
	/squelch /alias /PetWeapons /bc Distribute Pet Weapons
/if (${Debug}) /echo <== MAG_Aliases -|
/RETURN
|----------------------------------------------------------------------------|
SUB MAG_Background_Events
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
  /doevents auto_coh
  /doevents petWeapons
  /doevents armPets
/RETURN

|----------------------------------------------------------------------------|
| Destroy Orb of Mastery when it's out of charges and immediately summon a new one
| To enable:
| Add to MAG's Advanced Settings.ini
| [Magician]
| Auto-Summon Orb of Mastery (On/Off)=On
|----------------------------------------------------------------------------|
SUB check_OrbOfMasteryCharges 
  /if (!${autoOrbOfMastery}) /return 
  | Nothing on the cursor
  /if (!${Assisting} && !${Me.CombatState.Equal[Combat]} && !${Bool[${Cursor.ID}]}) {
    | if orb of mastery has 0 charges
    /if (${Bool[${FindItem[=Orb of Mastery]}]} && ${FindItem[=Orb of Mastery].Charges} == 0 ) {
      /docommand ${ChatToggle} Orb of Mastery is out of charges, destroying it
      /call SwapItem "Orb of Mastery" "Cursor"
      | double check that the item on the cursor is the Orb of Mastery
      /if (${Cursor.Name.Equal[Orb of Mastery]}) {
        /destroy
      }
    }
    | if toon has no orb of mastery
    /if (!${Bool[${FindItem[=Orb of Mastery]}]}) {
      /docommand ${ChatToggle} No Orb of Mastery in inventory, summoning an Orb of Mastery
      /casting "Summon Orb"
      /delay 12s ${Cursor.ID}
      /while (${Cursor.ID}) {
        /autoinv
        /delay 1s
      }
    }    
  }  
/RETURN
