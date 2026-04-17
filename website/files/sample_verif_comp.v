From Stdlib Require Import List.
Require Extraction.
Import ListNotations.

(*
 * Syntaxe d'un λ-calcul simplifié à l'arithmetique et la 
 * λ-abstraction
 *)

(* Utilités *)
Definition binop_opt {A} (op : A -> A -> A) (lhs rhs : option A) : option A :=
  match lhs, rhs with
  | None, _ => None
  | _, None => None
  | Some l, Some r => (Some (op l r))
  end
  .

Extraction binop_opt.

Notation "a +? b" := (binop_opt (fun a b => a + b) a b) (at level 70).
Notation "a -? b" := (binop_opt (fun a b => a - b) a b) (at level 70).


(*
  L'on définit ici l'arithmetique de base avec des variables
*)

Section lambda_calc.

  Inductive arith_expr : Type :=
    | const (n : nat)
    | var (id : nat)
    | add : arith_expr -> arith_expr -> arith_expr 
    | sub : arith_expr -> arith_expr -> arith_expr 
    .

  Fixpoint interp_arith (ctx : list (option nat)) (e : arith_expr) : option nat :=
    match e with
    | const n => (Some n)
    | var id => List.nth id ctx None
    | add e1 e2 => let lhs := (interp_arith ctx e1) in 
                   let rhs := (interp_arith ctx e2) in
                   lhs +? rhs
    | sub e1 e2 => let lhs := (interp_arith ctx e1) in 
                   let rhs := (interp_arith ctx e2) in
                   lhs -? rhs
    end.

  (*
   * On calcule par exemple ici (1 - x) + y avec x = 1 et y = 2
  *)
  Compute (
    let ctx := [Some 2 ; Some 1] in 
      interp_arith ctx (add 
        (sub (const 1) (var 1)) 
        (var 0)
      )
  ).

  Inductive lambda_expr : Type :=
    | arith : arith_expr -> lambda_expr 
    | lambda : lambda_expr -> lambda_expr
    | app : lambda_expr -> lambda_expr -> lambda_expr 
    .

  (*
   * La variable la plus récente est à la tête de la liste
   * Par exemple, pour λx.λy.x+y, le contexte serait [y;x]
   * (Voir les indices de De Bruijn)
   *)
  Fixpoint interp_lambda (ctx : list (option nat)) (e : lambda_expr) : option nat :=
    match e with
    | arith e' => interp_arith ctx e'
    | lambda e' => interp_lambda ctx e'
    | app efun eapplied => 
        let applied_res := interp_lambda ctx eapplied in
        let ctx' := applied_res::ctx in
        interp_lambda ctx' efun
    end.

  (*
   * On calcule ici λx.λy.x-y 5 (λx.x-3 9)
  *)
  Compute (
    interp_lambda [] (
      app 
        (app 
          (lambda (lambda (arith (sub (var 1) (var 0)))))
          (arith (const 5))
        )
        (app
          (lambda (arith (sub (var 0) (const 3))))
          (arith (const 9))
        )
    )
  )
  .

End lambda_calc.

(*
 * Le langage source et son comportement a été défini.
 * On passe maintenant au langage cible, un langage d'assemblage
 * proche de celui de la JVM
 *)

Section asm.

  Inductive instruction : Type :=
    (* Manipulation de la pile *)
    | put (n : nat)   (* Rajoute une constante au sommet *)
    | pop             (* Supprimme le sommet *)
    (* Manipulation des variables *)
    | save (n : nat)  (* Sauvegarde une constante à l'id 0*)
    | load (id : nat) (* Accède à l'id correspondant *)
    | remember        (* Sauvegarde et dépile la tête de pile *)
    | forget          (* Oublie la variable à l'id 0*)
    (* Arithmétique *)
    | plus            (* Dépilent les deux nombres au sommet, *)
    | minus           (* et placent le résultat au sommet *)
    .

  Fixpoint interp_asm 
    (prog : list instruction)
    (stack : list (option nat)) 
    (vars : list (option nat))
    : option nat :=
    match prog with
    | []               => List.nth 0 stack None
    | (put n)::prog'   => interp_asm prog' ((Some n)::stack) vars
    | pop::prog'       => interp_asm prog' (List.tl stack) vars
    | (save n)::prog'  => interp_asm prog' stack ((Some n)::vars)
    | (load id)::prog' => let res := List.nth id vars None in
                          interp_asm prog' (res::stack) vars
    | remember::prog'  => interp_asm prog' 
                            (List.tl stack) 
                            ((List.hd None stack)::vars)
    | forget::prog'    => interp_asm prog' stack (List.tl vars)
    | plus::prog'      => match stack with
                          | a::b::stack' => 
                              interp_asm prog' ((a +? b)::stack') vars
                          | _ => None
                          end
    | minus::prog'     => match stack with
                          | a::b::stack' =>
                              interp_asm prog' ((a -? b)::stack') vars
                          | _ => None
                          end
    end.

  Compute interp_asm
    [
      save 5;
      save 3;
      load 0;
      put 3;
      minus;
      forget;
      load 0;
      plus
    ]
    [] [].

End asm.

(* 
 * Maintenant qu'on a bien tout défini, l'on peut enfin envisager de
 * compiler notre λ-calcul en langage d'assemblage
 *)

Section compile.

  (* L'on compile d'abord les expressions arithmétiques *)
  Fixpoint comp_arith (source : arith_expr) : list instruction :=
    match source with
    | const n => [put n]
    | var id => [load id]
    | add lhs rhs => 
        (comp_arith rhs) ++ 
        (comp_arith lhs) ++ 
        [plus]
    | sub lhs rhs =>
        (comp_arith rhs) ++
        (comp_arith lhs) ++
        [minus]
    end.

  (* Pour se convaincre que ça fonctionne : on prend 3 + (2 - 1)*)
  Definition ex_arith :=
    (add
      (const 3)
      (sub
        (const 2)
        (const 1)
      )
    )
  .

  Compute comp_arith ex_arith.
  
  Compute interp_arith [] ex_arith.

  Compute interp_asm (comp_arith ex_arith) [] [].

  (* Enfin, l'on passe au λ-calcul propre *)
  Fixpoint comp_lambda (source : lambda_expr) : list instruction :=
    match source with
    | arith e => comp_arith e
    | lambda e => comp_lambda e
    | app e1 e2 =>
        (comp_lambda e2) ++
        [remember] ++
        (comp_lambda e1) ++
        [forget]
    end.

  (* Pour se convaincre, on compile (λx.λy. x+y+y+x) 3 1 *)
  Definition ex_lambda :=
    app
      (app
        (lambda (lambda (arith (
          add 
            (add (var 1) (var 0)) 
            (add (var 0) (var 1))
        ))))
        (arith (const 3))
      )
      (arith (const 1))
      .

  Compute comp_lambda ex_lambda.

  Compute interp_lambda [] ex_lambda.

  Compute interp_asm (comp_lambda ex_lambda) [] [].

End compile.

(* On a maintenant tout ce qu'il nous faut pour commencer à
 * vérifier que la compilation respecte bien la spécification
 * donnée précédemment
 *)

Section verification.

  (* L'on vérifie dans un premier temps la compilation de 
   * l'arithmétique
   *)
  Theorem arith_verif_attempt :
    forall (source : arith_expr) (result : option nat),
    interp_arith [] source = result -> 
      interp_asm (comp_arith source) [] [] = result.
  Proof.
    intros source result source_interp.
    induction source as 
      [| 
       | lhs ind_hyp_on_lhs rhs ind_hyp_on_rhs 
       | lhs ind_hyp_on_lhs rhs ind_hyp_on_rhs
      ].
    - simpl in *.
      apply source_interp.
    - simpl in *.
      assumption.
    - simpl in *.
      (* Ah, on aimerait pouvoir dire quelque chose 
         sur l'interprétation de la concaténation de listes
         d'instructions... *)
  Abort.

  Fail Lemma asm_concat_interp:
    forall 
      (source1 source2 : list instruction) 
      (stack_init stack_after1 : list (option nat))
      (vars_init vars_after1 : list (option nat)),
    interp_asm source1 stack_init vars_init -> 
    (* euh... comment on récupère l'état de la machine en milieu
     * d'éxécution ? *)
    interp_asm (source1 ++ source2) stack_init vars_init =
    interp_asm source2 stack_after1 vars_after1.
  Fail Proof.
  Fail Abort.

  (* Du coup il nous faut une extension de l'interpréteur assembleur
   * qui retourne en plus les états de la pile et des variables...
   *)

  Record machine_state := {
    stack : list (option nat);
    vars : list (option nat)
  }.

  Fixpoint interp_asm_full 
    (prog : list instruction)
    (stack : list (option nat)) 
    (vars : list (option nat))
    : machine_state :=
    match prog with
    | []               => {|stack := stack ;
                            vars := vars|}
    | (put n)::prog'   => interp_asm_full prog' ((Some n)::stack) vars
    | pop::prog'       => interp_asm_full prog' (List.tl stack) vars
    | (save n)::prog'  => interp_asm_full prog' stack ((Some n)::vars)
    | (load id)::prog' => let res := List.nth id vars None in
                          interp_asm_full prog' (res::stack) vars
    | remember::prog'  => interp_asm_full prog' 
                            (List.tl stack) 
                            ((List.hd None stack)::vars)
    | forget::prog'    => interp_asm_full prog' stack (List.tl vars)
    | plus::prog'      => match stack with
                          | a::b::stack' => 
                              interp_asm_full prog' ((a +? b)::stack') vars
                          | _ => {|stack := stack ; 
                                   vars := vars|}
                          end
    | minus::prog'     => match stack with
                          | a::b::stack' =>
                              interp_asm_full prog' ((a -? b)::stack') vars
                          | _ => {|stack := stack ; 
                                   vars := vars|}
                          end
    end.

  (* Maintenant que c'est fait, peut être qu'on peut ENFIN travailler sur la preuve
   * elle-même !
   *)

  Lemma asm_interp_concat :
    forall 
      (source1 source2 : list instruction) 
      (stack_init stack_after1 : list (option nat))
      (vars_init vars_after1 : list (option nat)),
    interp_asm_full source1 stack_init vars_init = 
    {|stack := stack_after1 ; vars := vars_after1|} ->
    interp_asm_full (source1 ++ source2) stack_init vars_init =
    interp_asm_full source2 stack_after1 vars_after1.
  Proof.
    intros.
    induction source2.
    - simpl. 
      rewrite app_nil_r.
      assumption.
    - induction a.
      + (* Eh bah non! Il nous manque le cas pour le constructeur! *)
  Admitted.

  (* Pour gagner du temps, on admet que c'est vrai. En pratique, ça compromet
   * beacoup l'intégrité de Rocq, puisque ça ouvre la possibilité d'admettre
   * quelque-chose de faux
   *)

  Lemma id_proj_machine (m : machine_state) :
    m = {| stack := stack (m) ; vars := vars (m)|}.
  Proof.
    destruct m as [s v].
    auto.
  Qed.

  Theorem arith_verif :
    forall (source : arith_expr) (result res1 res2 : option nat),
    interp_arith [] source = result -> 
      (List.hd None (interp_asm_full (comp_arith source) [] []).(stack)) = result.
  Proof.
    intros source result res1 res2 h.
    induction source in h, result |- *.
    1,2: assumption.
    - specialize (IHsource1 res1) ; specialize (IHsource2 res2).
      pose (res1 +? res2 = result) as sum.
      simpl.
      pose (interp_asm_full (comp_arith source2) [] []) as asm2_state.
      rewrite (asm_interp_concat 
        (comp_arith source2) 
        (comp_arith (source1) ++ [plus]) 
        [] 
        (asm2_state.(stack)) 
        [] 
        (asm2_state.(vars)) 
      ) ; cycle 1.
      { apply id_proj_machine. }
      pose (interp_asm_full 
        (comp_arith source1) asm2_state.(stack) asm2_state.(vars)) as asm1_state.
      rewrite (asm_interp_concat 
        (comp_arith source1) 
        ([plus]) 
        (stack (asm2_state))
        (asm1_state.(stack)) 
        (vars (asm2_state))
        (asm1_state.(vars))
      ) ; cycle 1.
      { apply id_proj_machine. }
      (* On triche : on admet ces résultats sans les prouver, 
       * en se convaincant que c'est vrai pour se faire une conscience *)
      assert (stack asm1_state = res1::res2::[]) as H ; cycle 1.
      + rewrite H.
        simpl.
        assert (res1 +? res2 = result) as true_sum ; cycle 1.
        * rewrite <- true_sum.
          reflexivity.
      (* La preuve est pratiquement la même pour la soustraction *)
  Abort.

End verification.
