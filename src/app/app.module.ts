import { PdfService } from './services/pdf.service';
import { BrowserModule } from '@angular/platform-browser';
import { FormsModule } from '@angular/forms';
import { NgModule } from '@angular/core';
import { NgxPrintModule } from 'ngx-print';
import { AppRoutingModule } from './app-routing.module';
import { AppComponent } from './app.component';
import { ConnectionService } from 'ng-connection-service';
import { OnlineStatusModule } from 'ngx-online-status';
import { MomentModule } from 'ngx-moment';
import { GethtmlService } from './services/gethtml/gethtml.service';

@NgModule({
  declarations: [
    AppComponent
  ],
  imports: [
    MomentModule,
    BrowserModule,
    AppRoutingModule,
    NgxPrintModule,
    FormsModule,
    OnlineStatusModule,
  ],
  providers: [ConnectionService, PdfService, GethtmlService],
  bootstrap: [AppComponent]
})
export class AppModule { }
