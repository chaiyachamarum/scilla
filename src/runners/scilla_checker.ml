(*
 * Copyright (c) 2018 - present Zilliqa, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *)


open Printf
open Sexplib.Std
open Syntax
open Core
open DebugMessage
open TypeChecker
open MonadUtil
open Result.Let_syntax

let check_parsing ctr = 
    let parse_module =
      FrontEndParser.parse_file ScillaParser.cmodule ctr in
    match parse_module with
    | None -> fail (sprintf "%s\n" "Failed to parse input file.")
    | Some cmod -> 
        plog @@ sprintf
          "\n[Parsing]:\nContract module [%s] is successfully parsed.\n" ctr;
        pout (sprintf "%s\n" (JSON.ContractInfo.get_string cmod.contr));
        pure cmod

let check_typing cmod elibs =
  let res = type_module cmod elibs in
  match res with
  | Error msg -> pout @@ sprintf "\n%s\n\n" msg; res
  | Ok _ ->
      let cn = get_id cmod.cname in 
        plog @@ sprintf
          "\n[Type Checking]:\nContract module [%s] is well-typed.\n"
          cn;
        res

let () =
  if (Array.length Sys.argv) <> 2
  then
    (perr (sprintf "Usage: %s foo.scilla\n" Sys.argv.(0))
    )
  else (
    GlobalConfig.set_debug_level GlobalConfig.Debug_None;
    (* Testsuite runs this executable with cwd=tests and ends
       up complaining about missing _build directory for logger.
       So disable the logger. *)
    let _ = (
      let%bind cmod = check_parsing Sys.argv.(1) in
      let elibs = {lname= mk_ident "Dummy"; lentries = []} in
      check_typing cmod elibs)
    in ())
