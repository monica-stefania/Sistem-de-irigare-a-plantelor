## Tematica proiectului:
Sistem inteligent pentru monitorizarea și irigarea automată a plantelor
 
## Descriere:
Proiectul propune dezvoltarea unui sistem embedded capabil să monitorizeze în timp real nivelul de umiditate din solul unei plante și să intervină automat pentru irigarea acesteia atunci când este necesar. Sistemul utilizează un microcontroler Raspberry Pi Pico 2W, valorificând funcționalitățile acestuia de conversie analog-digitală (ADC) asistată de acces direct la memorie (DMA), gestionare a evenimentelor asincrone prin întreruperi și comunicație prin protocolul I2C. Sistemul oferă și o interfață utilizator prin intermediul display-ului OLED pentru a putea vedea nivelul de umiditate al solului și, cu ajutorul unui buton vom putea declanșa ciclul de irigare la acționarea acestuia, iar alt buton va avea rol de "Kill Switch" în cazul în care pompa de apă se blochează sau dorim oprirea umezirii solului. 

## Componente utilizate:
- Raspberry Pi Pico 2W  
- Senzor capacitiv de umiditate  
- Modul releu  
- Pompă submersibilă  
- Display OLED 0.96" (I2C)  
- Butoane tactile

## Cerințe funcționale:
- Monitorizarea solului: Sistemul trebuie să citească periodic valorile analogice de la senzorul capacitiv de umiditate folosind perifericul ADC și tehnologia DMA, convertind valoarea brută într-un procentaj (0-100%).

- Irigare automată: Sistemul trebuie să acționeze un modul releu pentru a porni o pompă de apă submersibilă atunci când umiditatea măsurată scade sub un prag prestabilit

- Afișare display: Sistemul trebuie să afișeze în timp real, pe un ecran OLED 0.96" comunicând prin protocolul I2C, nivelul de umiditate și starea curentă a pompei (ON/OFF)

- Acționare manual și siguranță: Sistemul trebuie să integreze două butoane tactile, gestionate prin întreruperi hardware cu răspuns instantaneu: un buton pentru declanșarea forțată a ciclului de irigare și un buton cu rol de oprire de urgență (Kill Switch), care să blocheze imediat funcționarea pompei indiferent de starea sistemului.

## Cerințe non-funcționale:
- Siguranță și Izolare Electrică: Circuitul logic de control (3.3V) al microcontrolerului nu este conectat direct la elementul de execuție (pompa de 5V), acestea sunt legate prin intermediul unui modul releu, pentru a preveni defectarea plăcii de dezvoltare.

- Fiabilitate Hardware: Măsurarea umidității solului se va realiza folosind exclusiv un senzor de tip capacitiv, evitându-se senzorii rezistivi pentru a preveni coroziunea, ceea ce ar conduce la distrugerea rapidă a electrozilor prin electroliză.

- Protecție la inundație: Odată declanșată pompa (automat sau manual), aceasta va funcționa pentru un timp limitat, urmat de un timp de repaus în care sistemul nu va mai iriga, chiar dacă pragul este scăzut. Aceasta permite apei să se infiltreze în sol și previne inundarea plantei.

- Reactivitate și Latență: Sistemul trebuie să asigure un timp de răspuns minim la acționarea butonului de oprire de urgență (Kill Switch)

## Scenarii de testare:

### 1. Test monitorizare umiditate și irigare automată a solului
**Pași:**
- Se introduce senzorul în sol  
- Se observă valorile afișate pe display  

**Rezultat așteptat:**
- Pe display apare procentul de umiditate al solului (0-100%)  
- Pompa pornește automat dacă valoarea de pe display este sub un anumit prag (ex: 30%)  

### 2. Test acționare manuală a butonului
**Pași:**
- Apasă butonul de irigare  

**Rezultat așteptat:**
- Pompa pornește indiferent de umiditate  

### 3. Test Kill Switch
**Pași:**
- Pornește pompa  
- Apasă butonul de oprire de urgență  

**Rezultat așteptat:**
- Pompa se oprește din irigat  

### 4. Verificarea fiabilității sistemului
**Pași:**
- Lasă sistemul sa ruleze o perioadă îndelungată de timp  

**Rezultat așteptat:**
- Nu apar blocaje  
- Nu se strică vreo componentă a sistemului  
