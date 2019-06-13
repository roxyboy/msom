/**
   Energy dynagnostics of the MSQG model

   We multiply all terms of the PV equation by po/f
   We also multiply the entire equation by 1/f
*/

scalar * de_bfl = NULL;
scalar * de_vdl = NULL;
scalar * de_j1l = NULL;
scalar * de_j2l = NULL;
scalar * de_j3l = NULL;
scalar * de_ftl = NULL;
scalar * tmp2l = NULL;

trace
void jacobian_de(scalar po, scalar qo, scalar jac, double add, 
                 scalar po_r, double dt)
{
  foreach()
    jac[] = add*jac[] + 
    (( qo[1, 0 ]-qo[-1, 0])*(po[0, 1]-po[ 0 ,-1])
     +(qo[0 ,-1]-qo[ 0 ,1])*(po[1, 0]-po[-1, 0 ])
     + qo[1, 0 ]*( po[1,1 ] - po[1,-1 ])
     - qo[-1, 0]*( po[-1,1] - po[-1,-1])
     - qo[ 0 ,1]*( po[1,1 ] - po[-1,1 ])
     + qo[0 ,-1]*( po[1,-1] - po[-1,-1])
     + po[ 0 ,1]*( qo[1,1 ] - qo[-1,1 ])
     - po[0 ,-1]*( qo[1,-1] - qo[-1,-1])
     - po[1, 0 ]*( qo[1,1 ] - qo[1,-1 ])
     + po[-1, 0]*( qo[-1,1] - qo[-1,-1]))
    *Ro[]/(12.*Delta*Delta)
    *po_r[]*Ro[]*dt*Ro[];
}


trace
void advection_de  (scalar * qol, scalar * pol, 
                    scalar * de_j1l, scalar * de_j2l, scalar * de_j3l, double dt)
{  
  for (int l = 0; l < nl ; l++) {
    scalar qo  = qol[l];
    scalar po  = pol[l];
    scalar pp  = ppl[l];
    scalar de_j1 = de_j1l[l];
    scalar de_j2 = de_j2l[l];
    jacobian_de(po, qo, de_j1, 1., po, dt); // -J(p_qg, zeta)
    jacobian_de(pp, qo, de_j2, 1., po, dt); // -J(p_pg, zeta)
  }
  
   for (int l = 0; l < nl-1 ; l++) {
    scalar po1  = pol[l];
    scalar po2  = pol[l+1];
    scalar jac1 = tmpl[l];
    jacobian_de(po1, po2, jac1, 0., po1, dt); // -J(p_l, p_l+1)
  }
 combine_jac(tmpl, de_j1l, 1., 1. , 1.);

   for (int l = 0; l < nl-1 ; l++) {
    scalar po1  = pol[l];
    scalar pp1  = ppl[l];
    scalar po2  = pol[l+1];
    scalar jac1 = tmpl[l];
    jacobian_de(pp1, po2, jac1, 0., po1, dt); // -J(pp_l, p_l+1)
  }
 combine_jac(tmpl, de_j2l, 1., 0., 1.);
 combine_jac(tmpl, de_j3l, 1., 1., 0.);


   for (int l = 0; l < nl-1 ; l++) {
    scalar po1  = pol[l];
    scalar jac1 = tmpl[l];
    scalar pp2  = ppl[l+1];
    jacobian_de(po1, pp2, jac1, 0., po1, dt); // -J(p_l, pp_l+1)
  }
 combine_jac(tmpl, de_j2l, 1., 1., 0.);
 combine_jac(tmpl, de_j3l, 1., 0., 1.);
}


trace
void dissip_de  (scalar * zetal, scalar * dqol, scalar * pol, double dt)
{
  double iRe = 1/Re;
  double iRe4 = -1/Re4;
  comp_del2(zetal, tmpl, 0., 1.);

  foreach() 
    for (int l = 0; l < nl ; l++) {
      scalar dqo = dqol[l];
      scalar p4 = tmpl[l];
      scalar po = pol[l];
      dqo[] += p4[]*iRe*po[]*Ro[]*dt*Ro[];
      dqo[] += iRe4*(p4[1] + p4[-1] + p4[0,1] + p4[0,-1] - 4*p4[])/(sq(Delta))
        *po[]*Ro[]*dt*Ro[];
    }
}

trace
void bottom_friction_de (scalar * zetal, scalar * dqol, scalar * pol, double dt)
{
  scalar dqo  = dqol[nl-1];
  scalar zeta = zetal[nl-1];
  scalar po   = pol[nl-1];
  foreach()
    dqo[] -= Ek*zeta[]*po[]*Ro[]*dt*Ro[];
}


trace
void filter_de (scalar * qol, scalar * pol, scalar * de_ftl)
{
  wavelet_filter(qol, pol, tmp2l, -1.0, 0);
  foreach()
    for (int l = 0; l < nl ; l++) {
      scalar de_ft = de_ftl[l];
      scalar tmp = tmp2l[l];
      scalar po = pol[l];
      de_ft[] += tmp[]*po[]*Ro[]*Ro[];
    }
}

void energy_tend (scalar * pol, double dt)
{
  comp_del2(pol, zetal, 0., 1.0);
  advection_de(zetal, pol, de_j1l, de_j2l, de_j3l, dt);
  dissip_de(zetal, de_vdl, pol, dt);
  bottom_friction_de(zetal, de_bfl, pol, dt);
  filter_de (qol, pol, de_ftl);
}

void set_vars_energy(){
  de_bfl = create_layer_var(de_bfl,nl);
  de_vdl = create_layer_var(de_vdl,nl);
  de_j1l = create_layer_var(de_j1l,nl);
  de_j2l = create_layer_var(de_j2l,nl);
  de_j3l = create_layer_var(de_j3l,nl);
  de_ftl = create_layer_var(de_ftl,nl);
  tmp2l  = create_layer_var(tmp2l, nl);
}

void trash_vars_energy(){
  free(de_bfl), de_bfl = NULL;
  free(de_vdl), de_vdl = NULL;
  free(de_j1l), de_j1l = NULL;
  free(de_j2l), de_j2l = NULL;
  free(de_j3l), de_j3l = NULL;
  free(de_ftl), de_ftl = NULL;
  free(tmp2l), tmp2l = NULL;
}

event defaults (i = 0){
 if (ediag) set_vars_energy();
}
event cleanup (i = end, last) {
 if (ediag) trash_vars_energy();
}

event comp_diag (i++) {
 if (ediag) energy_tend (pol, dt);
}