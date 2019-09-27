import { TestBed } from '@angular/core/testing';

import { GethtmlService } from './gethtml.service';

describe('GethtmlService', () => {
  beforeEach(() => TestBed.configureTestingModule({}));

  it('should be created', () => {
    const service: GethtmlService = TestBed.get(GethtmlService);
    expect(service).toBeTruthy();
  });
});
