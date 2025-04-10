/// <reference path="../types/websocket.d.ts" />
import { createWebsocketHandoff } from 'fastly:websocket';
import { expectType } from 'tsd';

expectType<(request: Request, backend: string) => Response>(
  createWebsocketHandoff,
);
