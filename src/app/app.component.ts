import { PdfService } from './services/pdf.service';
import { Component, OnInit } from '@angular/core';
import * as jsPDF from 'jspdf';
import * as imagens from './services/imagens';
import { Notificado } from './models/notificado';
import * as moment from 'moment-timezone';
import * as $ from 'jquery';
import { Observable, Observer } from 'rxjs';
const googleUrl = 'https://chart.googleapis.com/chart?chs=150x150&cht=qr&chl=https://https://ccuapi.herokuapp.com/resposta/';


@Component({
  selector: 'app-root',
  templateUrl: './app.component.html',
  styleUrls: ['./app.component.css']
})
export class AppComponent implements OnInit {
  notificado: Notificado = {
    nome: 'NIERALDO DA SILVA LIMA',
    endereco: 'ESTRADA DO COLÉGIO',
    numero: '1245',
    complemento: '1245, APT 101',
    municipio: 'RIO DE JANEIRO',
    bairro: 'COLÉGIO',
    cep: '21235-280',
    infracao: '90909',
    data: this.gerarData(),
    dataTexto: this.pdfservice.getDataExtenso(this.gerarData()),
    qrcode: ''
  };
  base64Image: string;



  constructor(private pdfservice: PdfService) { }

  downloadPDF() {
    // tslint:disable-next-line: max-line-length
    this.notificado.qrcode = googleUrl + this.notificado.infracao;
    return this.pdfservice.downloadPDF(this.notificado);
  }

  gerarData() {
    const data = Date.now();
    const dateMoment = moment(data);
    return dateMoment.tz('America/Sao_Paulo').format('DD/MM/YYYY');
  }


  ngOnInit(): void {

      $.ajax({
        crossOrigin: true,
        url: 'http://scca.rio.rj.gov.br/index.php/online?im=80040075',
        headers: {
          'Access-Control-Allow-Origin': '*'
        },
        success(data) {
          // $('.chat').append(data.responseText);
          // $('.chat').append(data.find('svg'));
          $('.teste').append(data.body);
        }
      });
    }

    // // tslint:disable-next-line: only-arrow-functions
    // $.get('http://scca.rio.rj.gov.br/index.php/online?im=80048890').done((data) => {
    //   console.log(data);
    // });


}
