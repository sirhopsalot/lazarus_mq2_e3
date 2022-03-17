|----------------------------------------------------------------------------|
| Magician Functions
|----------------------------------------------------------------------------|
|#event petWeapons "#1# tells you, 'Pet Weapon#*#'"
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


#event armPets "<#1#> Pet Weapon#*#"
#event armPets "<#1#> Arm Pets"
#event armPets "<#1#> Arm Pets #2#"
#event armPets "#1# tells you, 'Arm Pet'"
#event armPets "#1# tells you, 'Arm Pets'"
#event armPets "#1# tells you, 'Pet Weapon#*#'"
#event armPets "#1# tells you, 'Pet Armor*#'"
#event armPets "#1# tells you, 'Arm Pet #2#'"
#event armPets "#1# tells you, 'Arm Pets #2#'"
sub event_armPets(line, ChatSender)
/if (${Debug} || ${Debug_MAG})  /echo event_armPets ==>

  | For some reason events dont like optional parameters, this is how to get the arguments after 'arm pets' in the tell
  /declare characterToWorkOn string local
  /declare loadType string local
  /if (${line.Find[tells you]}) {
    /if (${line.Token[6, ].Length}>0) /varset loadType ${line.Token[6, ]}
    /if (${line.Token[7, ].Length}>0) /varset characterToWorkOn ${line.Token[7, ].Left[-1]}
  } else {
    /if (${line.Token[4, ].Length}>0) /varset loadType ${line.Token[4, ]}
    /if (${line.Token[5, ].Length}>0) /varset characterToWorkOn ${line.Token[5, ].Left[-1]}
  }
  /if (${Debug} || ${Debug_MAG}) /echo [${Time}] LoadType: ${loadType}, CharacterToWorkOn: ${characterToWorkOn}

  |first check that we have an empty slot
  /declare BackSlotThatIsOpen int 0
  /declare itemInSlotID int local 0
  /declare itemInSlot string local NULL
  /declare i int local

  /for i 1 to 10
  /if (!${Bool[${InvSlot[pack${i}].Item.ID}]}) {

    /varset BackSlotThatIsOpen ${i}
    /goto :bagslotcheckdone

  } else /if (!${Bool[${InvSlot[pack${i}].Item.Container}]}) {

    /varset BackSlotThatIsOpen ${i}
    /varset itemInSlotID ${i}
    /varset itemInSlot ${InvSlot[pack${i}].Item.Name}
    /goto :bagslotcheckdone

  } else /if (${InvSlot[pack${i}].Item.Name.Equal[Phantom Satchel]} || ${InvSlot[pack${i}].Item.Name.Equal[Pouch of Quellious]}) {

    /call findAndDestroyOldSatchels
    /if (!${Bool[${InvSlot[pack${i}].Item.ID}]}) {
      |/echo Pack slot ${i}
      /varset BackSlotThatIsOpen ${i}
      /goto :bagslotcheckdone
    } else {
      /if (${Bool[${InvSlot[pack${itemInSlotID}].Item.ID}]}) {
        /echo [${Time}]: "Arm Pets: Bag slot not cleared. Exiting"
        /goto :armpetend
      }
    }
  }
  /next i
  :bagslotcheckdone

  | If an item was found in a pack slot, move the item to the first available slot inside a bag
  /if (${itemInSlotID} > 0) {
    /declare m int local 1
    /declare n int local 1
    /for m 1 to 10
    /for n 1 to 10 
    /if (!${InvSlot[pack${m}].Item.Item[${n}].ID}) {
      /itemnotify pack${itemInSlotID} leftmouseup
      /itemnotify in pack${m} ${n} leftmouseup
      /goto :bagslotcleared
    }
    /next n
    /next m
    :bagslotcleared
    /if (${Cursor.ID}) {
      /tell ${ChatSender}  Arm Pets: Unable to clear bagslot. Exiting
      /goto :armpetend
    }
    /if (${Bool[${InvSlot[pack${itemInSlotID}].Item.ID}]}) {
      /echo [${Time}]: "Arm Pets: Bag slot not cleared. Exiting"
      /goto :armpetend
    }
  }

  /if (${BackSlotThatIsOpen}==0) {
    |no empty bag slot available
    /tell ${ChatSender}  Arm Pets: No empty bag slot on mage. Exiting.
    /goto :armpetend
  } 
 
  /tell ${ChatSender} Starting to arm pets. Stand close.

  | When finished, will try to move mage back to their original location
  /declare meY float local ${Me.Y}
  /declare meX float local ${Me.X}

  /if (${Cursor.ID}) {
    /echo [${Time}]: Arm Pets: Something is on your cursor, exiting.
    | delays are needed as back to back tells fail to send
    /delay 3s 
    /tell ${ChatSender}  Arm Pets: Error, check mage for detail.
    /goto :armpetend
  }

  | Determine if player is in my group
  /if (${Group.Leader.ID}>0) {
    /declare g int local
    /declare memberOfMyGroup bool local
    /for g 0 to ${Group.Members}
      /if (${ChatSender.Equal[${Spawn[${Group.Member[${g}].Name}]}]}) /varset memberOfMyGroup true
    /next g
  }

  |/echo Character to work on is: ${characterToWorkOn}
  /declare groupCounter int local
  /if (${Bool[${line.Find[pet weapon]}]} || ${Bool[${line.Find[pet armor]}]} || !${loadType.Length}>0) { 
    | For compatibility, if player calls pet weapon it will only give armor/heirloom
    /echo [${Time}]: Pet Weapon was called, only handing armor/heirloom to pet
    /if (${Spawn[${ChatSender}].Pet.Level}<45) {
        /call armOnePet ${ChatSender} ${ChatSender} lowerpet
    } else {
      /delay 3s
      /tell ${ChatSender}  Either Pet Weapon was called or you did not specify a loadout, only givng Armor and Heirlooms to your pet.
      /call armOnePet ${ChatSender} ${ChatSender} noweapon
    }

  } else /if (${characterToWorkOn.Length} > 0) {

    | User passed in a comma separated list of toons to work on
    /if (${characterToWorkOn.Find[,]}) {

      /declare currentChar string local

      /for groupCounter 1 to ${Math.Calc[${characterToWorkOn.Count[,]}+1]}

      /varset currentChar ${characterToWorkOn.Arg[${groupCounter},,]}
      |/echo [${Time}]: Working on ${Spawn[${characterToWorkOn.Arg[${groupCounter},,]}].Name}
      /if (${Spawn[${currentChar}].ID}>0) {
        /call armOnePet ${currentChar} ${ChatSender} ${loadType}
        /if (${Cursor.ID}) {
          /bc Arm Pets: Something went wrong and something is on your cursor, exiting.
          /goto :armpetend
        } 
      } else {
        /delay 3s
        /tell ${ChatSender} Could not locate ${currentChar}, skipping.
      }
      /next groupCounter

    } else {
      | Character name was provided, arm only their pet
      |/echo Have one character to work on. Char: ${characterToWorkOn} Spawn ID: ${Spawn[${characterToWorkOn}].ID}
      /if (${Spawn[${characterToWorkOn}].ID}>0) {
        |/echo [${Time}]: found Character to work on.

        /call armOnePet ${characterToWorkOn} ${ChatSender} ${loadType} 
      } else {
        /delay 3s
        |/echo [${Time}]: Didnt find character ${characterToWorkOn}
        /tell ${ChatSender}  Could not locate character ${characterToWorkOn}
      }
    }

  } else /if (${memberOfMyGroup}) {
    | Character is a member of my group, arm all pets in group
    /for groupCounter 0 to ${Group.Members}
    /call armOnePet ${Spawn[${Group.Member[${groupCounter}].Name}]} ${ChatSender} ${loadType} 
    /if (${Cursor.ID}) {
      /bc Arm Pets: Something went wrong and something is on your cursor, exiting.
      /goto :armpetend
    }    
    /next groupCounter

  } else { 
    | When the chat sender is not a member, arm only their pet
    |/echo [${Time}]: Arming ${ChatSender} 's Pet
    /call armOnePet ${ChatSender} ${ChatSender} ${loadType} 
  }

  /call findAndDestroyOldSatchels
  /if (${itemInSlotID} > 0) {
    |/echo [${Time}]: Item to put back ${itemInSlot}
    /if (${FindItemCount[${itemInSlot}]} > 0) {
      /itemnotify "${itemInSlot}" leftmouseup
    } else {
      |/echo [${Time}]: Could not find item to put back ${itemInSlot}
      /if (${itemInSlot.Equal[Folded Pack of Spectral Armaments]}) {
        /call castSimpleSpell "Grant Spectral Armaments" ${Me.ID}
      } else /if (${itemInSlot.Equal[Folded Pack of Enibik's Heirlooms]}) {
        /call castSimpleSpell "Grant Enibik's Heirlooms" ${Me.ID}
      } else /if (${itemInSlot.Equal[Folded Pack of Spectral Plate]}) {
        /call castSimpleSpell "Grant Spectral Plate" ${Me.ID}
      }
    }
    /delay 3s ${Bool[${Cursor.ID}]}
    /call ClearCursor
  }
  /if (${Cursor.ID}) {
    /bc Arm Pets: Something went wrong and something is on your cursor, exiting.
    /goto :armpetend
  }

  /delay 3s
  /tell ${ChatSender} Finished arming pets.
  :armpetend
  /moveto loc ${meY} ${meX}

/if (${Debug} || ${Debug_MAG}) /echo <== event_armPets -|
/return

sub armOnePet(playerName, chatSender, loadoutType)

    /declare fireWeapon string local "Summoned: Fist of Flame"
    /declare iceWeapon string local "Summoned: Orb of Chilling Water"
    /declare malaWeapon string local "Summoned: Spear of Maliciousness"
    /declare slowWeapon string local "Summoned: Mace of Temporal Distortion"
    /declare dispelWeapon string local "Summoned: Wand of Dismissal"
    /declare wardWeapon string local "Summoned: Short Sword of Warding"
    /declare drainingBuckler string local "Summoned: Buckler of Draining Defense"
    /declare clericHammer string local "Hammer of Damnation"

    /declare bodytype string local
    /declare char_class string local
    /declare primaryWeapon string local
    /declare secondaryWeapon string local 

    |/echo [${Time}]: Checking ${playerName} 's pet
    |/echo [${Time}]: ${Spawn[${playerName}].Name} the ${Spawn[${playerName}].Class} has pet: ${Spawn[${playerName}].Pet.ID} which is ${bodytype}
    /if (${Spawn[${playerName}].Pet.ID} && !${Spawn[${playerName}].Pet.Name.Find[familiar]} && ${Spawn[${playerName}].Pet.Level}>1) {

      /bc Arm Pets: Working on ${playerName} 's pet

      |/call TrueTarget ${Spawn[${playerName}].Pet.ID}
      |/echo [${Time}]: Pet distance is ${Spawn[${playerName}].Pet.Distance}
      /if (${Spawn[${playerName}].Pet.Distance}>50) {
        /echo [${Time}]: Pet is too far away.
        /delay 3s 
        /tell ${chatSender}  ${playerName}'s pet too far away.
        /return

      }
      /if (${Me.Book[Grant Enibik's Heirlooms]}) {
           /bc Arm Pets: Creating Heirlooms
           /call summonUnpackAndGive ${Spawn[${playerName}].Pet.ID} "Grant Enibik's Heirlooms" "Folded Pack of Enibik's Heirlooms"
      } 
      /if (${Cursor.ID}) {
        /bc Arm Pets: Something went wrong and something is on your cursor, exiting.
        /return
      }
      /if (${Me.Book[Grant Spectral Plate]}) {
        /bc Arm Pets: Creating Spectral Plate
        /call summonUnpackAndGive ${Spawn[${playerName}].Pet.ID} "Grant Spectral Plate" "Folded Pack of Spectral Plate"
      }
      /if (${Cursor.ID}) {
        /bc Arm Pets: Something went wrong and something is on your cursor, exiting.
        /return
      }
    
      
      |/echo [${Time}]: the line to parse is:${loadoutType}
      /if (${Me.Book[Grant Spectral Armaments]} && !${loadoutType.Find[none]} && !${loadoutType.Find[noweapon]}) {
        /varset char_class ${Spawn[${playerName}].Class} 
        |/echo [${Time}]: loadoutType: ${loadoutType}

        |/echo [${Time}]: Pet level is ${Spawn[${playerName}].Pet.Level}
        /if (${Spawn[${playerName}].Pet.Level}<45) {
          | This will rely on gimme setup to get hammers from your cleric, otherwise we can just give the buckler
          |/echo [${Time}]: Pet is below 45, giving buckler and cleric hammer.

          /varset secondaryWeapon ${drainingBuckler}
          /if (${FindItemCount[${clericHammer}]}>0) {
            /varset primaryWeapon ${clericHammer}
          } else {
            /varset primaryWeapon NOPRIMARY
            /delay 3s
            /tell ${chatSender} Pet ${Spawn[${playerName}].Pet.CleanName} is below level 45, you should ask a cleric for a Pet Hammer when I am finished.
          }

        } else /if (${loadoutType.Find[,]} || ${loadoutType.Find[2]}) {
          |/echo [${Time}]: Token1 ${loadoutType.Token[1,,]} Token2 ${loadoutType.Token[2,,]} 
          /declare firstItem string local
          /declare secondItem string local

          /if (${loadoutType.Find[2]}) {
            |/echo [${Time}]: Setting loadout type for same primary/secondary: ${loadoutType},${loadoutType}
            /varset loadoutType ${loadoutType},${loadoutType}
          }
          
          /if (${loadoutType.Token[1,,].Length}>0) {
            /varset firstItem ${loadoutType.Token[1,,]}

            /if (${firstItem.Find[fire]}) {
              /varset primaryWeapon ${fireWeapon}

            } else /if (${firstItem.Find[ice]}) {
              /varset primaryWeapon ${iceWeapon}

            } else /if (${firstItem.Find[slow]}) {
              /varset primaryWeapon ${slowWeapon}

            } else /if (${firstItem.Find[mala]}) {
              /varset primaryWeapon ${malaWeapon}

            } else /if (${firstItem.Find[dispel]}) {
              /varset primaryWeapon ${dispelWeapon}

            } else /if (${firstItem.Find[ward]}) {
              /varset primaryWeapon ${wardWeapon}

            } else {
              /delay 3s
              /tell ${chatSender}  Invalid primary weapon specified, skipping weapon step.
              /return
            }
          }

          /if (${loadoutType.Token[2,,].Length}>0) {
            /varset secondItem ${loadoutType.Token[2,,]}

            /if (${secondItem.Find[fire]}) {
              /varset secondaryWeapon ${fireWeapon}

            } else /if (${secondItem.Find[ice]}) {
              /varset secondaryWeapon ${iceWeapon} 

            } else /if (${secondItem.Find[mala]}) {
              /varset secondaryWeapon ${malaWeapon}

            } else /if (${secondItem.Find[dispel]}) {
              /varset secondaryWeapon ${dispelWeapon}

            } else /if (${secondItem.Find[slow]}) {
              /varset secondaryWeapon ${slowWeapon}

            } else /if (${secondItem.Find[shield]}) {
              /varset secondaryWeapon ${drainingBuckler}

            } else {
              /delay 3s
              /tell ${chatSender}  Invalid secondary weapon specified, skipping weapon step.
              /return
            }
          }

        } else /if (${loadoutType.Find[default]} || ${loadoutType.Find[class]}) {

            /if (${char_class.Find[Magician]} || ${char_class.Find[Necro]} || ${char_class.Find[Beastlord]}) {
              |/echo [${Time}]: Arm Pet: Doing ${char_class} loadout
              /if (${loadoutType.Find[ice]}) {
                /varset primaryWeapon ${iceWeapon}
                /varset secondaryWeapon ${iceWeapon}
                
              } else {
                /varset primaryWeapon ${fireWeapon}
                /varset secondaryWeapon ${fireWeapon}
              }
            } else {
              |When its any other class with pet
              |/echo [${Time}]: loadoutType: ${loadoutType}
              /echo Arm Pet: Doing Default ${char_class} loadout
              
              /varset primaryWeapon ${dispelWeapon}
              /varset secondaryWeapon ${malaWeapon}
            }
        } else {
          /delay 3s 
          /tell ${chatSender}  Invalid loadout specified, skipping weapon step.
          /return
        }

        /bc Arm Pets: Selected Weapons are ${primaryWeapon} and ${secondaryWeapon}
        /bc Arm Pets: Creating Weapons

        /call summonUnpackAndSelect ${Spawn[${playerName}].Pet.ID} "Grant Spectral Armaments" "Folded Pack of Spectral Armaments" ${primaryWeapon} ${secondaryWeapon}
      }
    }
    

/return

sub summonUnpackAndSelect(targetID, spellName, ItemName, primaryItem, secondaryItem)

        /declare clericHammer string local "Hammer of Damnation"
        /declare drainingBuckler string local "Summoned: Buckler of Draining Defense"
        /if (${FindItemCount[${ItemName}]} > 0) {
          |/echo [${Time}]: found ${ItemName}
          /itemnotify "${ItemName}" leftmouseup
        } else {
          /call castSimpleSpell "${spellName}" ${Me.ID}
          /delay 3s ${Bool[${Cursor.ID}]}
        }

        |/echo [${Time}]: done casting, put in inventory
        /call ClearCursor
        /delay 3s !${Bool[${Cursor.ID}]}
        /delay 1s
        |/echo [${Time}]: unfolding the bag to cursor
        /ItemNotify "${ItemName}" rightmouseup
        /delay 3s ${Bool[${Cursor.ID}]}
        /call ClearCursor
        /delay 3s !${Bool[${Cursor.ID}]}
        /delay 1s

        | If its a cleric hammer and mage does not have one already, just skip the primary section
        |/if (!${primaryItem.Find[${clericHammer}]} || (${primaryItem.Find[${clericHammer}]} && ${FindItemCount[${clericHammer}]}>0)) {
         /if (!${primaryItem.Equal[NOPRIMARY]}) {
          /echo [${Time}]: Giving Primary Weapon to pet: ${primaryItem}
          /itemnotify "${primaryItem}" leftmouseup
          /delay 5s ${Bool[${Cursor.ID}]}
          /if (!${Cursor.ID}) {
            /echo [${Time}]: summonUnpackAndSelect: Could not load primary item to cursor, exiting
            /return
          }
          |/echo giving primary item to pet
          /call TrueTarget ${targetID}
          /if (${Target.ID} && ${Target.Distance}<50) {
            /call MoveToLoc ${Target.Y} ${Target.X} 50 15
            /call giveItemOnCursorToTarget
          } else {
            /echo [${Time}]: Pet is too far away.
            /return
          }
        }

        /if (${Cursor.ID}) {
          /echo [${Time}]: summonUnpackAndSelect: Something went wrong and something is on your cursor, exiting.
          /return
        }

        |check to see if we have a 2ndary item
        /if (${secondaryItem.Length}>0) {
          /if (!${Bool[${FindItem[${secondaryItem}]}]}) {
          
          |we don't have the 2ndary item in this pack, summon the pack again and try again
          |now delete the object
            /itemnotify "Pouch of Quellious" leftmouseup
            /delay 5s ${Bool[${Cursor.ID}]}
            /if (${Cursor.Name.Find[Pouch of Quellious]}) {
              /destroy
            } else {
              /echo [${Time}]: summonUnpackAndSelect: Could not destroy bag, exiting
              /return
            }
            
            /if (${FindItemCount[${ItemName}]} > 0) {
              |/echo found ${ItemName}
              /itemnotify "${ItemName}" leftmouseup
            } else {
              /call castSimpleSpell "${spellName}" ${Me.ID}
              /delay 3s ${Bool[${Cursor.ID}]}
            }

            |/echo done casting, put in inventory
            /call ClearCursor
            /delay 3s !${Bool[${Cursor.ID}]}
            |/echo unfolding the bag to cursor
            /ItemNotify "${ItemName}" rightmouseup
            /delay 3s ${Bool[${Cursor.ID}]}
            /call ClearCursor
            /delay 3s !${Bool[${Cursor.ID}]}
            /call TrueTarget ${targetID}
          } 
        
          |2ndary item
          /if (${secondaryItem.Find[${drainingBuckler}]}) {
            | for some reason itemnotify cant get the buckler by name or invslot, have to look for its pack slot
            /echo [${Time}]: Giving buckler to pet: ${secondaryItem}
            /declare i int local
            /for i 1 to 10
            /if (${Bool[${InvSlot[pack${i}].Item.Name.Equal[Pouch of Quellious]}]}) {
              /itemnotify in pack${i} 5 leftmouseup
              /goto :pouchFound
            }
            /next i
            :pouchFound
          } else {
            /echo [${Time}]: Giving Secondary weapon to pet: ${secondaryItem}
            /itemnotify "${secondaryItem}" leftmouseup
          }
          
          /delay 5s ${Bool[${Cursor.ID}]}
          
          /if (!${Cursor.ID}) {
            /echo [${Time}]: summonUnpackAndSelect: Could not load secondary item to cursor.
            /return
          }
          
          |/echo giving secondary item to pet
          /call TrueTarget ${targetID}
          /if (${Target.ID} && ${Target.Distance}<50) {
            /call MoveToLoc ${Target.Y} ${Target.X} 50 15
            /call giveItemOnCursorToTarget
          } else {
            /echo [${Time}]: Pet is too far away.
            /return
          }
        }
        /if (${Cursor.ID}) {
          /echo [${Time}]: summonUnpackAndSelect: Something went wrong and something is on your cursor.
          /return
        }
      
        |now delete the object
        /itemnotify "Pouch of Quellious" leftmouseup
        /delay 5s ${Bool[${Cursor.ID}]}
        /if (${Cursor.Name.Find[Pouch of Quellious]}) {
           /destroy
        } else {
          /echo [${Time}]: summonUnpackAndSelect: Could not destroy bag.
          /return
        }

/return
sub findAndDestroyOldSatchels()

    /echo [${Time}]: Finding and destroying opened summoned packs
    | Just in case mage has been looting and items ended up in an old phantom pack
    /call ClearCursor
    /if (${Cursor.ID}) {
      /echo [${Time}]: findAndDestroyOldSatchels: Unable to clear cursor.
      /return
    }
    /declare m int local
    /declare n int local
    /for m 1 to 10
    /if (${InvSlot[pack${m}].Item.Name.Equal[Phantom Satchel]} || ${InvSlot[pack${m}].Item.Name.Equal[Pouch of Quellious]}) {
      /for n 1 to 10
      |/echo Item ${InvSlot[pack${m}].Item.Item[${n}].Name}
      /if (!${InvSlot[pack${m}].Item.Item[${n}].NoRent} && ${InvSlot[pack${m}].Item.Item[${n}].ID} > 0) {
        /echo [${Time}]: findAndDestroyOldSatchels: Non-Temporary Item ${InvSlot[pack${m}].Item.Item[${n}]} found in ${InvSlot[pack${m}].Item.Name}.
        /return
      }
      /next n
      /itemnotify pack${m} leftmouseup
      /if (${Cursor.Name.Find[Phantom Satchel]} || ${Cursor.Name.Find[Pouch of Quellious]}) {
        /destroy
        /if (${Cursor.ID}) {
          /echo [${Time}]: findAndDestroyOldSatchels: Unable to clear bagslot.
          /return
        }
      }
    }
    /next m
  /return

sub summonUnpackAndGive(targetID, spellName, ItemName)

    /if (${FindItemCount[${ItemName}]} > 0) {
      |/echo ItemFound: ${ItemName}
      /itemnotify "${ItemName}" leftmouseup
    } else {
      /call castSimpleSpell "${spellName}" ${Me.ID}
      /delay 3s ${Bool[${Cursor.ID}]}
    }

    /delay 1s
    |/echo done casting, put in inventory
    /autoinventory
    /delay 3s !${Bool[${Cursor.ID}]}
    /delay 1s
    |/echo unfolding the bag to cursor
    /ItemNotify "${ItemName}" rightmouseup
    /delay 3s ${Bool[${Cursor.ID}]}
    |/echo getting the target of the pet
    /call TrueTarget ${targetID}
    /if (${Target.ID} && ${Target.Distance}<50) {
      /call MoveToLoc ${Target.Y} ${Target.X} 50 15
      /call giveItemOnCursorToTarget
    } else {
      /echo [${Time}]: Pet is too far away.
      /return
    }
    /if (${Cursor.ID}) {
      /echo [${Time}]: summonUnpackAndGive: Something went wrong and something is on your cursor, exiting.
      /return
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
          /echo [${Time}]: Failed to open trade with ${Target.CleanName}.
        }
    } else {
        :WaitAccept_Loop
        /notify GiveWnd GVW_Give_Button LeftMouseUp
        /delay 1s !${Window[GiveWnd].Open}
        /if (${Window[GiveWnd].Open}) {
          /if (${retryWeapTimer}) {
            /goto :WaitAccept_Loop
          } else {
            /echo [${Time}]: Failed to open trade with ${Target.CleanName}.
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
      /call castSimpleSpell "Summon Orb" 0
      /delay 12s ${Cursor.ID}
      /while (${Cursor.ID}) {
        /autoinv
        /delay 1s
      }
    }    
  }  
/RETURN
